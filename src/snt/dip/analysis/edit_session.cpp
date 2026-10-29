#include <snt/dip/edit_session.h>

#include <atomic>
#include <cerrno>
#include <fstream>
#include <snt/dip/comparison.h>
#include <snt/dip/dip.h>
#include <snt/dip/inspect/diagnostic.h>
#include "../helpers.h"
#include "../parsers.h"
#include <algorithm>
#include <cctype>
#include <queue>
#include <stdexcept>
#include <system_error>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <stdio.h>
#include <sys/stdio.h>
#elif defined(__linux__)
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace snt::dip {
    namespace {

        std::filesystem::path checked_path(const std::string& spelling) {
            const std::filesystem::path path(spelling);
            if (spelling.empty() || path.is_absolute() || spelling.find('\\') != std::string::npos)
                throw std::invalid_argument("Edit path must be a nonempty project-relative path using '/'.");
            for (const auto& part : path)
                if (part == "." || part == "..")
                    throw std::invalid_argument("Edit path cannot contain '.' or '..' segments.");
            if (path.filename().empty())
                throw std::invalid_argument("Edit path must name a file.");
            return path.lexically_normal();
        }

        const ProjectBundleFile& find_file(const ProjectBundle& bundle, const std::string& path) {
            for (const auto& file : bundle.files)
                if (file.path == path)
                    return file;
            throw std::invalid_argument("Document is not part of the packaged project: " + path);
        }

        Environment evaluate(const ProjectBundle& bundle, const bool allow_unset_setup_values) {
            DIP parser;
            parser.add_project(bundle);
            return parser.parse(false, true, allow_unset_setup_values);
        }

        void validate_block(const BlockInput& input, const std::string& text) {
            if (input.kind == BlockInput::Kind::Array) {
                std::string normalized = text;
                normalized.erase(std::remove(normalized.begin(), normalized.end(), '\n'), normalized.end());
                trim(normalized);
                val::Array::StringType values;
                val::Array::ShapeType shape;
                const std::string parsed = parse_array(normalized, values, shape);
                const auto trailing = normalized.substr(parsed.size());
                if (std::any_of(trailing.begin(), trailing.end(), [](unsigned char ch) { return !std::isspace(ch); }))
                    throw std::invalid_argument("Unexpected text after the closing array bracket.");
                return;
            }

            std::queue<Line> lines;
            lines = parse_lines(lines, text, input.source_name);
            bool has_separator = false;
            auto remaining = lines;
            while (!remaining.empty()) {
                std::string line = remaining.front().code;
                remaining.pop();
                trim(line);
                if (line == "---") {
                    has_separator = true;
                    break;
                }
            }
            if (!has_separator)
                throw std::invalid_argument("Table block value is missing the `---` header separator.");
            ValueMetadata metadata;
            const auto columns = parse_table_nodes(lines, metadata, input.delimiter);
            if (columns.empty())
                throw std::invalid_argument("Table block value must define at least one column.");
        }

        std::filesystem::path canonical_or_absolute(const std::filesystem::path& path) {
            std::error_code error;
            const auto canonical = std::filesystem::weakly_canonical(path, error);
            if (!error)
                return canonical;
            error.clear();
            const auto absolute = std::filesystem::absolute(path, error);
            return error ? path.lexically_normal() : absolute.lexically_normal();
        }

        PreviewResult evaluate_candidate(const Environment& baseline, const ProjectBundle& candidate_bundle,
                                         const bool allow_unset_setup_values) {
            PreviewResult result;
            Environment candidate;
            result.baseline_valid = true;
            try {
                candidate = evaluate(candidate_bundle, allow_unset_setup_values);
                result.candidate_valid = true;
            } catch (const std::exception& error) {
                result.candidate_diagnostics.push_back(diagnostic_from_exception(error));
            }
            if (result.candidate_valid)
                result.comparison = compare(baseline, candidate);
            return result;
        }

        std::string read_file(const std::filesystem::path& path) {
            std::ifstream input(path, std::ios::binary);
            if (!input)
                throw std::runtime_error("Cannot read source document: " + path.string());
            std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
            if (input.bad())
                throw std::runtime_error("Failed while reading source document: " + path.string());
            return contents;
        }

        std::string without_override_registration(const std::string& manifest, const std::size_t file_line) {
            std::vector<std::size_t> starts{0};
            for (std::size_t index = 0; index < manifest.size(); ++index)
                if (manifest[index] == '\n' && index + 1 < manifest.size())
                    starts.push_back(index + 1);
            if (file_line < 2 || file_line > starts.size())
                throw std::runtime_error("The override registration line is outside the DIPfile.");
            const auto line_text = [&](const std::size_t index) {
                const auto first = starts[index];
                const auto last = index + 1 < starts.size() ? starts[index + 1] : manifest.size();
                std::string text = manifest.substr(first, last - first);
                trim(text);
                return text;
            };
            if (line_text(file_line - 2) != "overrides[]" ||
                line_text(file_line - 1).rfind("file", 0) != 0)
                throw std::runtime_error("The override registration has changed; reload before deleting it.");
            std::string result = manifest;
            const auto begin = starts[file_line - 2];
            const auto end = file_line < starts.size() ? starts[file_line] : manifest.size();
            result.erase(begin, end - begin);
            return result;
        }

        void atomic_write(const std::filesystem::path& target, const std::string& text) {
            static std::atomic<unsigned long long> sequence{0};
            const auto temporary = target.parent_path() /
                                   (target.filename().string() + ".snt-edit-" +
                                    std::to_string(sequence.fetch_add(1, std::memory_order_relaxed)) + ".tmp");
            try {
                {
                    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
                    if (!output)
                        throw std::runtime_error("Cannot create temporary source document: " + temporary.string());
                    output.write(text.data(), static_cast<std::streamsize>(text.size()));
                    output.flush();
                    if (!output)
                        throw std::runtime_error("Failed while writing temporary source document: " + temporary.string());
                }
                const auto permissions = std::filesystem::status(target).permissions();
                std::filesystem::permissions(temporary, permissions);
#ifdef _WIN32
                if (!MoveFileExW(
                        temporary.wstring().c_str(),
                        target.wstring().c_str(),
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
                    ))
                    throw std::filesystem::filesystem_error(
                        "Unable to atomically replace source document",
                        temporary,
                        target,
                        std::error_code(static_cast<int>(GetLastError()), std::system_category())
                    );
#else
                std::filesystem::rename(temporary, target);
#endif
            } catch (...) {
                std::error_code ignored;
                std::filesystem::remove(temporary, ignored);
                throw;
            }
        }

        void rename_new_directory(const std::filesystem::path& source, const std::filesystem::path& target) {
#ifdef _WIN32
            if (!MoveFileExW(source.wstring().c_str(), target.wstring().c_str(), MOVEFILE_WRITE_THROUGH))
                throw std::filesystem::filesystem_error(
                    "Unable to create Save As destination",
                    source,
                    target,
                    std::error_code(static_cast<int>(GetLastError()), std::system_category())
                );
#elif defined(__APPLE__)
            if (renamex_np(source.c_str(), target.c_str(), RENAME_EXCL) != 0)
                throw std::filesystem::filesystem_error(
                    "Unable to create Save As destination",
                    source,
                    target,
                    std::error_code(errno, std::generic_category())
                );
#elif defined(__linux__)
            if (syscall(SYS_renameat2, AT_FDCWD, source.c_str(), AT_FDCWD, target.c_str(), RENAME_NOREPLACE) != 0)
                throw std::filesystem::filesystem_error(
                    "Unable to create Save As destination",
                    source,
                    target,
                    std::error_code(errno, std::generic_category())
                );
#else
            if (std::filesystem::exists(target))
                throw std::filesystem::filesystem_error(
                    "Save As destination already exists", source, target, std::make_error_code(std::errc::file_exists)
                );
            std::filesystem::rename(source, target);
#endif
        }

    } // namespace

    EditSession::EditSession(const std::filesystem::path& project_file, const bool allow_unset_setup_values)
        : bundle_(package_project(project_file, {}, allow_unset_setup_values)),
          root_(std::filesystem::canonical(project_file).parent_path()),
          allow_unset_setup_values_(allow_unset_setup_values),
          baseline_(evaluate(bundle_, allow_unset_setup_values_)) {}

    bool EditSession::contains_document(const std::string& spelling) const {
        try {
            (void)find_file(bundle_, checked_path(spelling).generic_string());
            return true;
        } catch (const std::invalid_argument&) {
            return false;
        }
    }

    const std::string& EditSession::document_text(const std::string& spelling) const {
        return find_file(bundle_, checked_path(spelling).generic_string()).text;
    }

    PreviewResult EditSession::preview_document(const std::string& spelling, const std::string& text) const {
        const auto path = checked_path(spelling).generic_string();
        (void)find_file(bundle_, path);
        ProjectBundle candidate = bundle_;
        for (auto& file : candidate.files)
            if (file.path == path) {
                file.text = text;
                break;
            }
        return evaluate_candidate(baseline_, candidate, allow_unset_setup_values_);
    }

    PreviewResult EditSession::preview_block(const std::string& path, const std::string& text) const {
        PreviewResult result;
        result.baseline_valid = true;
        try {
            const auto input = baseline_.block_inputs().find(path);
            if (input == baseline_.block_inputs().end())
                throw std::invalid_argument("The selected block value is not available in this edit session.");
            validate_block(input->second, text);
            result.candidate_valid = true;
        } catch (const std::exception& error) {
            result.candidate_diagnostics.push_back(diagnostic_from_exception(error));
        }
        return result;
    }

    std::pair<std::string, std::string> EditSession::document_for_block(
        const std::string& path, const std::string& text
    ) const {
        const auto input = baseline_.block_inputs().find(path);
        if (input == baseline_.block_inputs().end())
            throw std::invalid_argument("The selected block value is no longer available.");
        const auto source = baseline_.sources.entries().find(input->second.source_name);
        if (source == baseline_.sources.entries().end())
            throw std::invalid_argument("The source document for this block value is unavailable.");

        std::filesystem::path source_path = source->second.path;
        std::size_t source_line = input->second.source_line;
        if (source->second.embedded) {
            const auto parent = baseline_.sources.entries().find(source->second.parent.name);
            if (parent == baseline_.sources.entries().end())
                throw std::invalid_argument("The parent document for this embedded block value is unavailable.");
            source_path = parent->second.path;
            source_line = source->second.parent.line_number;
        }
        if (source_path.empty())
            throw std::invalid_argument("The source document for this block value has no file path.");

        const auto source_absolute = source_path.is_absolute() ? source_path : root_ / source_path;
        const auto canonical_source = canonical_or_absolute(source_absolute);
        const ProjectBundleFile* document_file = nullptr;
        for (const auto& file : bundle_.files) {
            const auto bundled_path = canonical_or_absolute(root_ / file.path);
            if (bundled_path == canonical_source) {
                document_file = &file;
                break;
            }
        }
        if (!document_file)
            throw std::invalid_argument("The source document for this block value is not part of the project bundle.");

        std::string document = document_file->text;
        std::size_t offset = 0;
        for (std::size_t line = 1; line < source_line; ++line) {
            const auto newline = document.find('\n', offset);
            if (newline == std::string::npos)
                throw std::invalid_argument("The block value source line is outside its document.");
            offset = newline + 1;
        }
        const auto position = document.find(input->second.code, offset);
        if (position == std::string::npos)
            throw std::invalid_argument("Cannot locate the original block value in its source document.");
        document.replace(position, input->second.code.size(), text);
        return {document_file->path, std::move(document)};
    }

    void EditSession::save_block(const std::string& path, const std::string& text) {
        const auto replacement = document_for_block(path, text);
        save_document(replacement.first, replacement.second);
    }

    void EditSession::delete_override(const std::size_t project_entry_index) {
        const auto& entries = baseline_.project_entries();
        if (project_entry_index >= entries.size() || entries[project_entry_index].kind != ProjectEntry::Kind::Override ||
            entries[project_entry_index].resolved_path.empty())
            throw std::invalid_argument("The selected source is not a file-backed project override.");
        const auto& entry = entries[project_entry_index];
        const auto declared_file = std::filesystem::path(entry.value);
        const auto source_file = declared_file.is_absolute() ? declared_file : root_ / declared_file;
        const auto override_file = canonical_or_absolute(source_file);
        const auto relative = override_file.lexically_relative(root_);
        if (relative.empty() || relative.is_absolute() || *relative.begin() == ".." ||
            std::filesystem::is_symlink(source_file))
            throw std::invalid_argument("The override file must be a regular file inside the project directory.");
        const auto manifest_file = root_ / bundle_.entry;
        const auto& original_manifest = find_file(bundle_, bundle_.entry).text;
        const auto& original_override = find_file(bundle_, relative.generic_string()).text;
        const std::string edited_manifest = without_override_registration(original_manifest, entry.line);
        ProjectBundle candidate_bundle = bundle_;
        for (auto& file : candidate_bundle.files)
            if (file.path == bundle_.entry) {
                file.text = edited_manifest;
                break;
            }
        candidate_bundle.files.erase(
            std::remove_if(candidate_bundle.files.begin(), candidate_bundle.files.end(),
                           [&](const ProjectBundleFile& file) { return file.path == relative.generic_string(); }),
            candidate_bundle.files.end()
        );
        Environment candidate = evaluate(candidate_bundle, allow_unset_setup_values_);
        if (read_file(manifest_file) != original_manifest || read_file(override_file) != original_override)
            throw std::runtime_error("The DIPfile or override changed on disk; reload before deleting it.");
        atomic_write(manifest_file, edited_manifest);
        std::filesystem::remove(override_file);
        bundle_ = std::move(candidate_bundle);
        baseline_ = std::move(candidate);
    }

    void EditSession::save_document(const std::string& spelling, const std::string& text) {
        const auto relative = checked_path(spelling);
        const auto path = relative.generic_string();
        const auto& original = find_file(bundle_, path);
        ProjectBundle candidate_bundle = bundle_;
        for (auto& file : candidate_bundle.files)
            if (file.path == path) {
                file.text = text;
                break;
            }
        std::optional<Environment> candidate_environment;
        try {
            candidate_environment = evaluate(candidate_bundle, allow_unset_setup_values_);
        } catch (const std::exception&) {
            // Invalid text can still be saved; keep the last valid comparison baseline.
        }
        const auto target = root_ / relative;
        const auto resolved_parent = std::filesystem::canonical(target.parent_path());
        const auto parent_relative = resolved_parent.lexically_relative(root_);
        if (resolved_parent != root_ &&
            (parent_relative.empty() || parent_relative.is_absolute() ||
             (*parent_relative.begin() == "..")))
            throw std::invalid_argument("Edit target resolves outside the project directory.");
        if (std::filesystem::is_symlink(target) || read_file(target) != original.text)
            throw std::runtime_error("Source document changed on disk since it was loaded; reload before saving.");
        atomic_write(target, text);
        for (auto& file : bundle_.files)
            if (file.path == path) {
                file.text = text;
                if (candidate_environment)
                    baseline_ = std::move(*candidate_environment);
                return;
            }
    }

    bool EditSession::save_as(
        const std::filesystem::path& destination_directory,
        const std::optional<std::string>& edited_path,
        const std::string& edited_text
    ) {
        if (destination_directory.empty() || destination_directory.filename().empty() ||
            destination_directory.filename() == "." || destination_directory.filename() == "..")
            throw std::invalid_argument("Save As requires a new project directory path.");
        const auto absolute_destination = std::filesystem::absolute(destination_directory).lexically_normal();
        const auto parent = std::filesystem::canonical(absolute_destination.parent_path());
        const auto destination = parent / absolute_destination.filename();
        if (std::filesystem::exists(destination) || std::filesystem::is_symlink(destination))
            throw std::runtime_error("Save As destination already exists: " + destination.string());

        ProjectBundle candidate_bundle = bundle_;
        if (edited_path) {
            const auto path = checked_path(*edited_path).generic_string();
            (void)find_file(candidate_bundle, path);
            for (auto& file : candidate_bundle.files)
                if (file.path == path) {
                    file.text = edited_text;
                    break;
                }
        }
        std::optional<Environment> candidate_environment;
        try {
            candidate_environment = evaluate(candidate_bundle, allow_unset_setup_values_);
        } catch (const std::exception&) {
            return false;
        }

        static std::atomic<unsigned long long> sequence{0};
        std::filesystem::path staging;
        bool created = false;
        for (int attempt = 0; attempt < 100 && !created; ++attempt) {
            staging = parent / ("." + destination.filename().string() + ".snt-save-" +
                                std::to_string(sequence.fetch_add(1, std::memory_order_relaxed)));
            std::error_code error;
            created = std::filesystem::create_directory(staging, error);
            if (error && error != std::errc::file_exists)
                throw std::filesystem::filesystem_error("Cannot create Save As staging directory", staging, error);
        }
        if (!created)
            throw std::runtime_error("Cannot reserve a temporary directory for Save As.");

        try {
            for (const auto& file : candidate_bundle.files) {
                const auto relative = checked_path(file.path);
                const auto output_path = staging / relative;
                std::filesystem::create_directories(output_path.parent_path());
                std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
                if (!output)
                    throw std::runtime_error("Cannot create project document: " + output_path.string());
                output.write(file.text.data(), static_cast<std::streamsize>(file.text.size()));
                output.flush();
                if (!output)
                    throw std::runtime_error("Failed while writing project document: " + output_path.string());
            }
            rename_new_directory(staging, destination);
        } catch (...) {
            std::error_code ignored;
            std::filesystem::remove_all(staging, ignored);
            throw;
        }

        bundle_ = std::move(candidate_bundle);
        root_ = destination;
        baseline_ = std::move(*candidate_environment);
        return true;
    }

} // namespace snt::dip
