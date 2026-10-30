#ifndef SNT_DIP_EDIT_SESSION_H
#define SNT_DIP_EDIT_SESSION_H

#include <filesystem>
#include <optional>
#include <snt/dip/environment.h>
#include <snt/dip/preview.h>
#include <snt/dip/project_bundle.h>
#include <string>
#include <utility>

namespace snt::dip {

    /**
     * A source editing session for a file-backed DIP project or standalone DIPL file.
     *
     * The session retains the project as an in-memory bundle. Candidate text is
     * evaluated by replaying that bundle, so relative includes and normal DIP
     * phase ordering are identical to a regular project load. The viewer can
     * use this API without implementing parsing, evaluation, or diff logic.
     */
    class EditSession {
      public:
        /** Load a project or standalone .dip/.dipl file for editing. */
        explicit EditSession(const std::filesystem::path& artifact, bool allow_unset_setup_values = false);

        /** Return the original packaged project. */
        const ProjectBundle& bundle() const { return bundle_; }

        /** Test whether a normalized project-relative path is in the bundle. */
        bool contains_document(const std::string& path) const;

        /** Return the current session text for a bundled document. */
        const std::string& document_text(const std::string& path) const;

        /**
         * Evaluate a candidate replacement for an existing project-relative
         * document, returning baseline/candidate diagnostics and a comparison.
         * The path must name a file already present in the packaged project.
         */
        PreviewResult preview_document(const std::string& path, const std::string& text) const;

        /** Validate edited text using the recorded array or table block format. */
        PreviewResult preview_block(const std::string& path, const std::string& text) const;

        /**
         * Write a document back only if its on-disk contents still match the
         * version loaded by this session. The path must be an existing bundled
         * document. After a successful write, the session's baseline is updated.
         */
        void save_document(const std::string& path, const std::string& text);

        /** Replace and save one block value inside its original bundled source document. */
        void save_block(const std::string& path, const std::string& text);

        /** Create a new project-root .dip source and register it as a code[] entry. */
        void create_code_source(const std::string& filename);

        /** Create a new project-root .dip source and register it as a named sources[] entry. */
        void create_named_source(const std::string& name, const std::string& filename);

        /** Delete one file-backed override and its DIPfile registration after validating the remaining project. */
        void delete_override(std::size_t project_entry_index);

        /**
         * Copy the bundled project into a new directory. The destination must
         * not exist. An optional document replacement is included in the copy.
         * Throws on filesystem errors; returns false without creating a copy
         * when its candidate project does not evaluate successfully.
         */
        bool save_as(
            const std::filesystem::path& destination_directory,
            const std::optional<std::string>& edited_path = std::nullopt,
            const std::string& edited_text = {}
        );

      private:
        void create_source(const std::string& filename, const std::optional<std::string>& name);
        std::pair<std::string, std::string> document_for_block(const std::string& path,
                                                               const std::string& text) const;
        ProjectBundle bundle_;
        std::filesystem::path root_;
        bool standalone_ = false;
        bool allow_unset_setup_values_ = false;
        Environment baseline_;
    };

} // namespace snt::dip

#endif
