#ifndef SNT_VIEW_GRAPH_VIEW_H
#define SNT_VIEW_GRAPH_VIEW_H

#include "viewer_model.h"

#include <cstddef>
#include <string>

namespace snt::view {

    /** Dependency neighborhood and page positions for the selected object. */
    struct GraphViewState {
        std::string path;                         ///< Selected browser object path.
        std::size_t revision = 0;                 ///< Model revision represented by this neighborhood.
        dip::DependencyNeighborhood neighborhood; ///< Recorded dependencies and readers for the selection.
        std::size_t dependency_start = 0;         ///< First dependency displayed on the current page.
        std::size_t reader_start = 0;             ///< First reader displayed on the current page.

        /** Reload the neighborhood when the model selection or revision changes. */
        void reset(const ViewerModel& model);
    };

    /** Draw dependency and reader links with bounded paging. */
    void draw_graph_view(ViewerModel& model, GraphViewState& state);

} // namespace snt::view

#endif
