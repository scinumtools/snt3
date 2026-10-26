#include <snt/dip/edit_session.h>

#include <atomic>
#include <cerrno>
#include <fstream>
#include <snt/dip/comparison.h>
#include <snt/dip/dip.h>
#include <snt/dip/inspect/diagnostic.h>
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

        Environment evaluate(const ProjectBundle& bundle) {
            DIP parser;
            parser.add_project(bundle);
            return parser.parse();
        }

        PreviewResult evaluate_candidate(const Environment& baseline, const ProjectBundle& candidate_bundle) {
            PreviewResult result;
            Environment candidate;
            result.baseline_valid = true;
            try {
                candidate = evaluate(candidate_bundle);
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

    EditSession::EditSession(const std::filesystem::path& project_file)
        : bundle_(package_project(project_file)),
          root_(std::filesystem::canonical(project_file).parent_path()),
          baseline_(evaluate(bundle_)) {}

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
        return evaluate_candidate(baseline_, candidate);
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
            candidate_environment = evaluate(candidate_bundle);
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
            candidate_environment = evaluate(candidate_bundle);
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
