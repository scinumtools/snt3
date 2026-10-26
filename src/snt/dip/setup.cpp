#include <snt/core/string_format.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/setup.h>

#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace snt::dip {

    namespace {

        std::string quote_dipl_string(const std::string& value) {
            std::string result = "\"";
            for (const char character : value) {
                if (character == '"')
                    result += "\\\"";
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

    } // namespace

    SetupSession::SetupSession(const std::filesystem::path& project_file)
        : project_file_(std::filesystem::absolute(project_file)) {
        DIP parser;
        parser.add_project(project_file_);
        const auto environment = parser.parse(false, false, true);
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

} // namespace snt::dip
