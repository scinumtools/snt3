/** @file
 * @brief Viewer semantic inspection and source panels.
 */
#ifndef SNT_VIEW_INSPECTOR_VIEW_H
#define SNT_VIEW_INSPECTOR_VIEW_H

#include "viewer_model.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace snt::view {

    class SourceView;

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
        std::vector<SourceTarget> source_targets; ///< Navigable source locations for the selection.
        std::string error;                        ///< Inspection failure shown by the panel.
    };

    /** Draw semantic details and source links, updating the source tab and scroll request on navigation. */
    void draw_inspector(
        ViewerModel& model,
        InspectorCache& cache,
        SourceView& source,
        bool& select_source_tab,
        bool& scroll_to_target,
        std::string& source_error
    );
    /** Draw the loaded source buffer and honor a pending scroll-to-target request. */
    void draw_source_view(
        const ViewerModel& model, const SourceView& source, bool& scroll_to_target, const std::string& error
    );

} // namespace snt::view

#endif
