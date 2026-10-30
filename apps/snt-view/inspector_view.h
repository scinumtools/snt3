/** @file
 * @brief Viewer semantic inspection and source panels.
 */
#ifndef SNT_VIEW_INSPECTOR_VIEW_H
#define SNT_VIEW_INSPECTOR_VIEW_H

#include "viewer_model.h"
#include "source_view.h"

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace snt::view {

    class NotificationCenter;

    /** Source buffer and navigation requests shared by the browser, inspector, and editor. */
    struct SourceNavigationState {
        SourceView source;
        std::string owner;
        std::optional<SourceTarget> pending_target;
        std::string error;
        std::string edit_report;
        bool select_tab = false;

        void clear();
        bool open_selected(ViewerModel& model);
    };

    /** Cache semantic inspection fields and source targets for a model selection. */
    struct InspectorCache {
        std::string path;         ///< Selected browser object path.
        std::size_t revision = 0; ///< Model revision used to populate these fields.
        bool valid = false;       ///< Whether cached inspection data may be reused.
        std::vector<std::pair<const char*, std::string>>
            fields;                               ///< Display labels paired with formatted semantic values.
        std::vector<std::string> reads;           ///< Paths read while evaluating the selected value.
        std::vector<std::string> readers;         ///< Paths whose evaluation read the selected value.
        std::vector<std::string> schema_values;   ///< Values associated with the selected schema.
        std::vector<std::pair<std::string, std::string>> source_nodes; ///< Source node labels and browser paths.
        std::vector<SourceTarget> source_targets; ///< Navigable source locations for the selection.
        std::string error;                        ///< Inspection failure shown by the panel.
    };

    /** Draw semantic details and source links. */
    void draw_inspector(
        ViewerModel& model,
        InspectorCache& cache,
        SourceNavigationState& navigation,
        NotificationCenter& notifications
    );
    /** Draw the loaded source buffer and honor a pending scroll-to-target request. */
    void draw_source_view(
        ViewerModel& model,
        SourceNavigationState& navigation,
        bool& request_save_as,
        NotificationCenter& notifications
    );

} // namespace snt::view

#endif
