#ifndef SNT_VIEW_GRAPH_VIEW_H
#define SNT_VIEW_GRAPH_VIEW_H

#include "viewer_model.h"

#include <cstddef>
#include <string>

namespace snt::view {

struct GraphViewState {
    std::string path;
    std::size_t revision = 0;
    dip::DependencyNeighborhood neighborhood;
    std::size_t dependency_start = 0;
    std::size_t reader_start = 0;

    void reset(const ViewerModel& model);
};

void draw_graph_view(ViewerModel& model, GraphViewState& state);

} // namespace snt::view

#endif
