#include <snt/core/string_format.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/setup.h>

#include <algorithm>
#include <atomic>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace snt::dip {

    namespace {

        std::string quote_dipl_string(const std::string& value) {
            std::string result = "\"";
            for (const char character : value) {
                if (character == '"')
                    result += "\\\"";
                else if (character == '\\')
                    result += "\\\\";
                else if (character == '\n')
                    result += "\\n";
                else if (character == '\r')
                    result += "\\n";
                else
                    result += character;
            }
            result += '"';
            return result;
        }

        std::string option_expression(const ValueNode::OptionStruct& option) {
            core::StringFormatType format;
            format.valuePrecision = std::numeric_limits<double>::max_digits10;
            std::string expression = option.value->to_string(format);
            if (!option.units_raw.empty())
                expression += " " + option.units_raw;
            return expression;
        }

        std::string read_text(const std::filesystem::path& path) {
            std::ifstream input(path, std::ios::binary);
            if (!input)
                throw std::runtime_error("Unable to read project file: " + path.string());
            return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        }

        std::string_view line_text(const std::string& text, const std::vector<std::size_t>& starts,
                                   const std::size_t index) {
            const auto first = starts.at(index);
            auto last = index + 1 < starts.size() ? starts[index + 1] : text.size();
            while (last > first && (text[last - 1] == '\n' || text[last - 1] == '\r'))
                --last;
            return std::string_view(text.data() + first, last - first);
        }

        void replace_file(const std::filesystem::path& target, const std::string& contents) {
            static std::atomic<unsigned long long> sequence{0};
            const auto temporary = target.parent_path() /
                                   (target.filename().string() + ".snt-setup-" +
                                    std::to_string(sequence.fetch_add(1, std::memory_order_relaxed)) + ".tmp");
            try {
                {
                    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
                    if (!output)
                        throw std::runtime_error("Unable to create temporary DIPfile: " + temporary.string());
                    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
                    output.flush();
                    if (!output)
                        throw std::runtime_error("Unable to write temporary DIPfile: " + temporary.string());
                }
                std::filesystem::permissions(temporary, std::filesystem::status(target).permissions());
#ifdef _WIN32
                if (!MoveFileExW(temporary.wstring().c_str(), target.wstring().c_str(),
                                 MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                    throw std::runtime_error("Unable to replace DIPfile: " + target.string());
#else
                std::filesystem::rename(temporary, target);
#endif
            } catch (...) {
                std::error_code ignored;
                std::filesystem::remove(temporary, ignored);
                throw;
            }
        }

    } // namespace

    SetupSession::SetupSession(const std::filesystem::path& project_file)
        : project_file_(std::filesystem::absolute(project_file)) {
        DIP parser;
        parser.add_project(project_file_);
        const auto environment = parser.parse(false, false, true);
        const auto project_directory = project_file_.parent_path();
        const std::string setup_header = "# Guided setup answers for " + project_file_.filename().string() + "\n";
        for (const auto& entry : environment.project_entries()) {
            if (entry.kind != ProjectEntry::Kind::Override || entry.resolved_path.empty())
                continue;
            const auto path = std::filesystem::absolute(entry.resolved_path).lexically_normal();
            if (path.parent_path() == project_directory && read_text(path).rfind(setup_header, 0) == 0)
                registered_overrides_.emplace_back(path, entry.line);
        }
        if (environment.nodes.size() == 0)
            return;
        const auto nodes = environment.request_group("?", RequestType::Reference);

        core::StringFormatType format;
        format.valuePrecision = std::numeric_limits<double>::max_digits10;
        for (const auto& node : nodes) {
            if (!node || !node->setup)
                continue;
            SetupQuestion question;
            question.path = node->path.name;
            question.description = node->metadata.description;
            question.type = node->dtype;
            question.array = !node->dimension.empty();
            question.has_value = static_cast<bool>(node->value);
            if (node->value)
                question.current_value = node->to_string(format);
            question.options.reserve(node->options.size());
            for (const auto& option : node->options) {
                question.options.push_back(
                    {option_expression(option), option.label.empty() ? option_expression(option) : option.label}
                );
            }
            questions_.push_back(std::move(question));
        }
    }

    const SetupQuestion& SetupSession::question(const std::string& path) const {
        for (const auto& item : questions_)
            if (item.path == path)
                return item;
        throw std::invalid_argument("No `!setup` parameter is registered at `" + path + "`.");
    }

    void SetupSession::answer(const std::string& path, const std::string& input) {
        const auto& item = question(path);
        if (!item.options.empty())
            throw std::invalid_argument("Choose one of the declared `!options` for `" + path + "`.");
        if (input.empty() && item.type != NodeDtype::String)
            throw std::invalid_argument("A value is required for `" + path + "`.");
        const auto previous = answers_.find(path);
        const bool had_previous = previous != answers_.end();
        const std::string previous_value = had_previous ? previous->second : std::string{};
        if (item.type == NodeDtype::String && !item.array)
            answers_[path] = quote_dipl_string(input);
        else
            answers_[path] = input;
        try {
            parse_answers(true);
        } catch (...) {
            if (had_previous)
                answers_[path] = previous_value;
            else
                answers_.erase(path);
            throw;
        }
    }

    void SetupSession::choose(const std::string& path, const std::size_t option_index) {
        const auto& item = question(path);
        if (option_index >= item.options.size())
            throw std::out_of_range("The selected option is outside the choices for `" + path + "`.");
        const auto previous = answers_.find(path);
        const bool had_previous = previous != answers_.end();
        const std::string previous_value = had_previous ? previous->second : std::string{};
        answers_[path] = item.options[option_index].value;
        try {
            parse_answers(true);
        } catch (...) {
            if (had_previous)
                answers_[path] = previous_value;
            else
                answers_.erase(path);
            throw;
        }
    }

    void SetupSession::answer_batch(const std::map<std::string, std::string>& answers,
                                    const std::map<std::string, std::size_t>& choices) {
        auto staged = answers_;
        for (const auto& [path, input] : answers) {
            const SetupQuestion* item = nullptr;
            try {
                item = &question(path);
            } catch (const std::exception& error) {
                throw SetupInputError(path, error.what());
            }
            if (!item->options.empty())
                throw SetupInputError(path, "Choose one of the declared `!options` for `" + path + "`.");
            if (input.empty() && item->type != NodeDtype::String)
                throw SetupInputError(path, "A value is required for `" + path + "`.");
            staged[path] = item->type == NodeDtype::String && !item->array ? quote_dipl_string(input) : input;
        }
        for (const auto& [path, index] : choices) {
            const SetupQuestion* item = nullptr;
            try {
                item = &question(path);
            } catch (const std::exception& error) {
                throw SetupInputError(path, error.what());
            }
            if (answers.count(path))
                throw SetupInputError(path, "The setup form supplied two values for `" + path + "`.");
            if (index >= item->options.size())
                throw SetupInputError(path, "The selected option is outside the choices for `" + path + "`.");
            staged[path] = item->options[index].value;
        }
        auto previous = std::move(answers_);
        answers_ = std::move(staged);
        try {
            preview();
        } catch (const std::exception& error) {
            std::string invalid_path;
            for (const auto& item : questions_)
                if (!item.has_value && !answers_.count(item.path)) {
                    invalid_path = item.path;
                    break;
                }
            if (invalid_path.empty()) {
                if (const auto* dip_error = dynamic_cast<const dip::Exception*>(&error)) {
                    const auto& location = dip_error->info().location;
                    if (location && location->source == "snt_setup_answers" && location->line > 0) {
                        std::size_t line = 1;
                        for (const auto& item : questions_) {
                            const auto answer = answers_.find(item.path);
                            if (answer == answers_.end())
                                continue;
                            const std::size_t next = line +
                                static_cast<std::size_t>(std::count(answer->second.begin(), answer->second.end(), '\n')) + 1;
                            if (location->line >= line && location->line < next) {
                                invalid_path = item.path;
                                break;
                            }
                            line = next;
                        }
                    }
                }
            }
            if (invalid_path.empty()) {
                for (const auto& item : questions_)
                    if (std::string(error.what()).find("`" + item.path + "`") != std::string::npos) {
                        invalid_path = item.path;
                        break;
                    }
            }
            if (invalid_path.empty()) {
                for (const auto& item : questions_) {
                    const auto answer = answers_.find(item.path);
                    if (answer == answers_.end())
                        continue;
                    SetupSession probe = *this;
                    probe.answers_ = previous;
                    probe.answers_[item.path] = answer->second;
                    try {
                        probe.parse_answers(true);
                    } catch (const std::exception&) {
                        invalid_path = item.path;
                        break;
                    }
                }
            }
            const std::string message = error.what();
            answers_ = std::move(previous);
            if (!invalid_path.empty())
                throw SetupInputError(invalid_path, message);
            throw;
        }
    }

    std::string SetupSession::answer_text(const bool allow_incomplete) const {
        std::ostringstream output;
        for (const auto& item : questions_) {
            const auto answer = answers_.find(item.path);
            if (answer != answers_.end())
                output << item.path << " = " << answer->second << '\n';
            else if (!item.has_value && !allow_incomplete)
                throw std::invalid_argument("No answer was provided for required setup parameter `" + item.path + "`.");
        }
        return output.str();
    }

    std::string SetupSession::override_text() const {
        return answer_text(false);
    }

    Environment SetupSession::parse_answers(const bool allow_incomplete) const {
        const std::string overrides = answer_text(allow_incomplete);
        DIP parser;
        parser.add_project(project_file_);
        if (!overrides.empty())
            parser.add_override_string(overrides, "snt_setup_answers");
        return parser.parse(false, false, allow_incomplete);
    }

    Environment SetupSession::preview() const {
        return parse_answers(false);
    }

    void SetupSession::save(const std::filesystem::path& output_file) const {
        if (output_file.empty())
            throw std::invalid_argument("An output file path is required.");
        const std::string overrides = override_text();
        preview();
        if (std::filesystem::exists(output_file))
            throw std::runtime_error("Refusing to overwrite existing setup file: " + output_file.string());
        std::ofstream stream(output_file, std::ios::binary);
        if (!stream)
            throw std::runtime_error("Unable to create setup override file: " + output_file.string());
        stream << "# Guided setup answers for " << project_file_.filename().string() << "\n";
        stream << overrides;
        if (!stream)
            throw std::runtime_error("Unable to write setup override file: " + output_file.string());
    }

    void SetupSession::save_and_register(const std::filesystem::path& output_file) const {
        const auto project = std::filesystem::absolute(project_file_).lexically_normal();
        const auto output = std::filesystem::absolute(output_file).lexically_normal();
        if (project.filename() != "DIPfile" || output.parent_path() != project.parent_path() ||
            output.filename().empty() || output.extension() != ".dip" || output == project)
            throw std::invalid_argument("Setup output must be a new .dip file beside the DIPfile.");
        if (std::filesystem::exists(output))
            throw std::runtime_error("Refusing to overwrite existing setup file: " + output.string());

        const std::string original = read_text(project);
        std::string manifest = original;
        if (!manifest.empty() && manifest.back() != '\n')
            manifest += '\n';
        manifest += "\noverrides[]\n  file = " + quote_dipl_string(output.filename().string()) + "\n";
        const std::string generated = "# Guided setup answers for " + project.filename().string() + "\n" +
                                      override_text();
        DIP candidate;
        candidate.set_project_file_reader([&](const std::filesystem::path& path) {
            const auto absolute = std::filesystem::absolute(path).lexically_normal();
            if (absolute == project)
                return manifest;
            if (absolute == output)
                return generated;
            return read_project_file(path);
        });
        candidate.add_project(project);
        candidate.parse();

        if (read_text(project) != original)
            throw std::runtime_error("DIPfile changed on disk during setup; reload before committing.");
        save(output);
        try {
            replace_file(project, manifest);
        } catch (...) {
            std::error_code ignored;
            std::filesystem::remove(output, ignored);
            throw;
        }
    }

    void SetupSession::remove_registered_overrides() const {
        if (registered_overrides_.empty())
            throw std::runtime_error("No guided setup override is registered in this project.");

        const auto project = std::filesystem::absolute(project_file_).lexically_normal();
        const std::string original = read_text(project);
        std::vector<std::size_t> starts{0};
        for (std::size_t index = 0; index < original.size(); ++index)
            if (original[index] == '\n' && index + 1 < original.size())
                starts.push_back(index + 1);

        std::vector<std::pair<std::size_t, std::size_t>> ranges;
        for (const auto& [file, line] : registered_overrides_) {
            if (line < 2 || line > starts.size() ||
                line_text(original, starts, line - 2) != "overrides[]" ||
                line_text(original, starts, line - 1) != "  file = " + quote_dipl_string(file.filename().string()))
                throw std::runtime_error("The guided setup registration changed; reload before removing it.");
            const auto begin = starts[line - 2];
            const auto end = line < starts.size() ? starts[line] : original.size();
            ranges.emplace_back(begin, end);
        }
        std::sort(ranges.begin(), ranges.end(), [](const auto& left, const auto& right) {
            return left.first > right.first;
        });
        std::string manifest = original;
        for (const auto& [begin, end] : ranges)
            manifest.erase(begin, end - begin);

        DIP candidate;
        candidate.set_project_file_reader([&](const std::filesystem::path& path) {
            if (std::filesystem::absolute(path).lexically_normal() == project)
                return manifest;
            return read_project_file(path);
        });
        candidate.add_project(project);
        candidate.parse(false, false, true);

        if (read_text(project) != original)
            throw std::runtime_error("DIPfile changed on disk during setup; reload before removing overrides.");
        replace_file(project, manifest);
        for (const auto& [file, line] : registered_overrides_) {
            (void)line;
            std::filesystem::remove(file);
        }
    }

} // namespace snt::dip
