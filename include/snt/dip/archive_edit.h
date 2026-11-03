#ifndef SNT_DIP_ARCHIVE_EDIT_H
#define SNT_DIP_ARCHIVE_EDIT_H

#include <filesystem>
#include <map>
#include <snt/dip/environment.h>
#include <snt/dip/preview.h>
#include <string>

namespace snt::dip {

    /**
     * Edit the source bytes embedded in a DIPH5 v3 archive.
     */
    class ArchiveEditSession {
      public:
        /**
         * Load an archive and retain its resolved environment as the baseline.
         */
        explicit ArchiveEditSession(std::filesystem::path archive);

        /**
         * Return the archived text identified by its source name.
         */
        const std::string& document_text(const std::string& name) const;
        /**
         * Evaluate one replacement using the archived inputs and report diagnostics.
         */
        PreviewResult preview_document(const std::string& name, const std::string& text) const;
        /**
         * Evaluate and atomically replace the archive with the new v3 environment.
         */
        void save_document(const std::string& name, const std::string& text);

      private:
        Environment evaluate(const std::string& name, const std::string& text) const;
        std::filesystem::path archive_; ///< DIPH5 file being edited.
        Environment baseline_;         ///< Resolved environment before an edit.
        std::map<std::string, std::string> source_paths_; ///< Original names remain usable after a reparse renumbers them.
    };

} // namespace snt::dip

#endif
