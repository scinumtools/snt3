#include <snt/dip/archive_edit.h>

#include <atomic>
#include <map>
#include <snt/dip/comparison.h>
#include <snt/dip/dip.h>
#include <snt/dip/inspect/diagnostic.h>
#include "../hdf5/io_private.h"
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace snt::dip {
    ArchiveEditSession::ArchiveEditSession(std::filesystem::path archive)
        : archive_(std::filesystem::absolute(std::move(archive))) {
        baseline_.load(archive_);
        for (const auto& source : baseline_.archived_sources())
            source_paths_.emplace(source.info.name, source.info.path);
        for (const auto& source : baseline_.applied_override_sources())
            source_paths_.emplace(source.name, source.path);
    }

    const std::string& ArchiveEditSession::document_text(const std::string& name) const {
        for (const auto& source : baseline_.archived_sources())
            if (source.info.name == name)
                return source.content;
        for (const auto& source : baseline_.applied_override_sources())
            if (source.name == name)
                return source.content;
        const auto old = source_paths_.find(name);
        if (old != source_paths_.end() && !old->second.empty()) {
            for (const auto& source : baseline_.archived_sources())
                if (source.info.path == old->second && source.role != "other")
                    return source.content;
            for (const auto& source : baseline_.applied_override_sources())
                if (source.path == old->second)
                    return source.content;
        }
        throw std::invalid_argument("Source is not archived: " + name);
    }

    Environment ArchiveEditSession::evaluate(const std::string& name, const std::string& text) const {
        const auto old = source_paths_.find(name);
        const auto target_path = old == source_paths_.end() ? std::string{} : old->second;
        std::map<std::filesystem::path, std::string> files;
        std::filesystem::path entry;
        for (const auto& source : baseline_.archived_sources()) {
            if (source.info.path.empty() || source.role == "other")
                continue;
            const auto path = std::filesystem::path(source.info.path).lexically_normal();
            if (path.filename() == "DIPfile")
                entry = path;
            const auto content = source.info.name == name ||
                (!target_path.empty() && source.info.path == target_path) ? text : source.content;
            const auto [it, inserted] = files.emplace(path, content);
            if (!inserted && it->second != content)
                throw std::runtime_error("Archive has conflicting content for source path: " + path.string());
        }
        for (const auto& source : baseline_.applied_override_sources()) {
            if (source.path.empty())
                continue;
            const auto path = std::filesystem::path(source.path).lexically_normal();
            files[path] = source.name == name || (!target_path.empty() && source.path == target_path)
                ? text : source.content;
        }
        DIP parser;
        parser.set_project_file_reader([files](const std::filesystem::path& requested) {
            const auto found = files.find(requested.lexically_normal());
            if (found == files.end())
                throw std::runtime_error("Archived project requires an unavailable file: " + requested.string());
            return found->second;
        });
        if (!entry.empty())
            parser.add_project(entry);
        else {
            const auto& sources = baseline_.archived_sources();
            if (sources.empty())
                throw std::runtime_error("Archive has no entry source to reevaluate.");
            if (sources.front().info.path.empty())
                parser.add_string(sources.front().info.name == name ? text : sources.front().content);
            else
                parser.add_file(sources.front().info.path);
        }
        return parser.parse(true, true, true);
    }

    PreviewResult ArchiveEditSession::preview_document(const std::string& name, const std::string& text) const {
        document_text(name); // Reject unknown source names before evaluating.
        PreviewResult result;
        result.baseline_valid = true;
        try {
            auto candidate = evaluate(name, text);
            result.candidate_valid = true;
            result.comparison = compare(baseline_, candidate);
        } catch (const std::exception& error) {
            result.candidate_diagnostics.push_back(diagnostic_from_exception(error));
        }
        return result;
    }

    void ArchiveEditSession::save_document(const std::string& name, const std::string& text) {
        {
            hdf5::detail::Id archive(H5Fopen(archive_.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose,
                                     "Unable to reopen DIPH5 archive before editing");
            if (hdf5::detail::required_utf8(archive, "_DIPH5_Profile") != "environment")
                throw std::runtime_error("Experiment records cannot be rewritten by the source editor.");
        }
        const auto candidate = evaluate(name, text);
        static std::atomic<unsigned long long> sequence{0};
        const auto temporary = archive_.parent_path() /
            (archive_.filename().string() + ".snt-edit-" +
             std::to_string(sequence.fetch_add(1, std::memory_order_relaxed)) + ".tmp");
        try {
            candidate.save(temporary);
#ifdef _WIN32
            if (!MoveFileExW(temporary.wstring().c_str(), archive_.wstring().c_str(),
                             MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                throw std::runtime_error("Cannot replace the DIPH5 archive after editing.");
#else
            std::filesystem::rename(temporary, archive_);
#endif
        } catch (...) {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            throw;
        }
        baseline_.load(archive_);
    }

} // namespace snt::dip
