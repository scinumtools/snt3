/** @file
 * @brief Viewer panel for direct relationships between DIPL sources.
 */
#ifndef SNT_VIEW_SOURCE_GRAPH_VIEW_H
#define SNT_VIEW_SOURCE_GRAPH_VIEW_H

#include "viewer_model.h"

namespace snt::view {

    struct SourceNavigationState;

    /** Whether the browser entry represents a source that can have recorded reads. */
    bool supports_source_graph(const ObjectInfo& object);

    /** Draw a vertical graph of direct source reads and navigate to clicked source lines. */
    void draw_source_graph_view(ViewerModel& model, SourceNavigationState& navigation);

} // namespace snt::view

#endif
