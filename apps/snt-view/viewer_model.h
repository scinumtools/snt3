/** @file
 * @brief Viewer environment ownership, browser hierarchy, and navigation.
 */
#ifndef SNT_VIEW_VIEWER_MODEL_H
#define SNT_VIEW_VIEWER_MODEL_H

#include "source_format.h"

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <snt/dip/edit_session.h>
#include <snt/dip/inspect/inspector.h>
#include <string>
#include <utility>
#include <vector>

namespace snt::view {

    /** Semantic roles represented by entries in the viewer browser. */
    enum class ObjectRole {
        Artifact,         ///< Input artifact root.
        Project,          ///< Evaluated source project.
        DIPfile,          ///< Project manifest.
        ManifestCategory, ///< Group of manifest entries.
        ManifestEntry,    ///< One project manifest entry.
        LocalSources,     ///< Sources loaded from local files.
        NamedSourceNodes, ///< Nodes contributed by named sources.
        NamedSourceNodeGroup, ///< One named source's nodes in the provenance browser.
        DIPfileSource,    ///< Editable project manifest document.
        CodeSource,       ///< Source code file.
        BlockSources,     ///< Group of embedded source blocks.
        BlockSource,      ///< One embedded source block.
        Sources,          ///< Source metadata collection.
        RawSources,       ///< Stored source text collection.
        Source,           ///< One recorded source.
        Overrides,        ///< Applied override collection.
        Override,         ///< One applied override.
        Schemas,          ///< Schema collection.
        Schema,           ///< One schema definition.
        Units,            ///< Unit definition collection.
        Unit,             ///< One unit definition.
        Path              ///< DIPL hierarchy entry.
    };

    /** Browser entry linking display hierarchy to an inspected environment object. */
    struct ObjectInfo {
        std::string path;                                       ///< Unique browser path of this entry.
        std::string label;                                      ///< Human-readable tree label.
        std::string parent;                                     ///< Parent entry path, empty for the root.
        std::vector<std::string> children;                      ///< Ordered child entry paths.
        bool has_value = false;                                 ///< Whether this entry represents an evaluated value.
        dip::Path::Kind hierarchy_kind = dip::Path::Kind::None; ///< DIPL hierarchy node kind.
        ObjectRole role = ObjectRole::Path;                     ///< Role used to select browser and inspector behavior.
        std::string source_name;                                ///< Environment source identifier when applicable.
        std::string node_path;                     ///< Underlying DIPL node path when different from the browser path.
        std::optional<std::size_t> manifest_index; ///< Index into the applicable project manifest collection.
        bool has_table = false;                    ///< Whether this entry exposes a tabular data view.
        std::string source_kind;                    ///< Source file or raw data kind shown as a browser icon.
    };

    /** Source location or embedded block that the inspector can open. */
    struct SourceTarget {
        std::string label;                        ///< Human-readable navigation label.
        std::filesystem::path file;               ///< Original source file to display.
        std::size_t line = 0;                     ///< One-based recorded source line; zero when unavailable.
        std::string source_name;                  ///< Source identifier for locating embedded text.
        SourceFormat format = SourceFormat::DIPL; ///< Highlighting mode for the source target.
        std::optional<std::string> block_path; ///< Embedded source block path, when text is stored in the environment.
    };

    /** Own an evaluated environment, its browser hierarchy, source editing, and navigation history. */
    class ViewerModel {
      public:
        /** Load the requested project, source file, or saved environment for viewing. */
        explicit ViewerModel(std::filesystem::path artifact);

        /** Reload the input and rebuild the browser; retain an error when loading fails. */
        bool reload();
        /** Select an existing browser path and record the navigation in history. */
        bool select(const std::string& path);
        /** Move to the previous selection when navigation history permits. */
        bool back();
        /** Move to the next selection when navigation history permits. */
        bool forward();
        /** Test whether a previous selection is available. */
        bool can_back() const { return history_index_ > 0; }
        /** Test whether a later selection is available. */
        bool can_forward() const { return history_index_ + 1 < history_.size(); }
        /** Find a browser entry by path; return null when absent. */
        const ObjectInfo* object(const std::string& path) const;
        /** Resolve the value node represented by a browser entry, if any. */
        dip::ValueNode::PointerType value_node(const ObjectInfo& object) const;
        /** Return the inspection path for array or table data represented by an entry. */
        std::string data_path(const ObjectInfo& object) const;
        /** Collect navigable source locations for the given entry. */
        std::vector<SourceTarget> source_targets(const ObjectInfo& object) const;
        /** Find the browser entry under Sources that represents a source target. */
        std::string source_object_path(const SourceTarget& target) const;
        /** Browser identity for a node contributed by a named source. */
        std::string named_source_node_path(const std::string& source, const std::string& node_path) const;
        /** Return a project-relative path if this source is a bundled editable document. */
        std::optional<std::string> editable_document_path(const std::filesystem::path& file) const;
        /** Return packaged text for an editable source document. */
        const std::string& document_text(const std::string& path) const;
        /** Preview candidate text through the core project parser and comparison engine. */
        dip::PreviewResult preview_document(const std::string& path, const std::string& text) const;
        /** Validate block text using its recorded array or table format. */
        dip::PreviewResult preview_block(const std::string& path, const std::string& text) const;
        /** Save an editable document and refresh the evaluated environment when possible. */
        bool save_document(const std::string& path, const std::string& text);
        /** Save an edited block value into its original document and refresh the environment. */
        bool save_block(const std::string& path, const std::string& text);
        /** Copy the current project and optionally include the active unsaved document. */
        bool save_as(
            const std::filesystem::path& destination_directory,
            const std::optional<std::string>& edited_path = std::nullopt,
            const std::string& edited_text = {}
        );
        /** Report whether the loaded artifact has a file-backed project session. */
        bool can_save_as() const { return static_cast<bool>(edit_session_); }
        /** Return the currently loaded environment for read-only inspection. */
        const dip::Environment& environment() const { return environment_; }
        /** Return the resolved input artifact path. */
        const std::filesystem::path& artifact() const { return artifact_; }
        /** Return the original input path supplied to the viewer. */
        const std::string& input_path() const { return input_path_; }
        /** Format a source path for display relative to the input when possible. */
        std::string display_file_path(const std::string& path) const;
        /** Return the selected browser path. */
        const std::string& selection() const { return selection_; }
        /** Return the current browser search query. */
        const std::string& search() const { return search_; }
        /** Replace the browser search query. */
        void set_search(std::string query) { search_ = std::move(query); }
        /** Return the most recent load failure message. */
        const std::string& error() const { return error_; }
        /** Report whether displayed data was retained after a reload failure. */
        bool stale() const { return stale_; }
        /** Return the revision used to invalidate view caches after a successful load. */
        std::size_t revision() const { return revision_; }

      private:
        /** Rebuild browser entries from the current environment and input metadata. */
        void rebuild_objects();
        std::string input_path_;                    ///< Original input argument.
        std::filesystem::path artifact_;            ///< Resolved artifact used for loading.
        dip::Environment environment_;              ///< Loaded environment retained for inspection.
        std::unique_ptr<dip::EditSession> edit_session_; ///< Core project editing session, when file-backed.
        std::map<std::string, ObjectInfo> objects_; ///< Browser hierarchy indexed by entry path.
        std::string selection_;                     ///< Currently selected entry path.
        std::string search_;                        ///< Browser filter query.
        std::string error_;                         ///< Latest reload failure message.
        bool stale_ = false;                        ///< True when the retained environment predates a failed reload.
        std::size_t revision_ = 0;                  ///< Counter incremented after successful reloads.
        std::vector<std::string> history_;          ///< Visited browser paths in navigation order.
        std::size_t history_index_ = 0;             ///< Current position within navigation history.
    };

} // namespace snt::view

#endif
