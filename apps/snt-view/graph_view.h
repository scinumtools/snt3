#ifndef SNT_VIEW_GRAPH_VIEW_H
#define SNT_VIEW_GRAPH_VIEW_H

#include "viewer_model.h"

#include <snt/exs/composition.h>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace snt::view {

struct GraphNeighbor {
    std::string id;
    std::string request;
    std::string operand;
};

struct GraphViewState {
    std::string path;
    std::size_t revision = 0;
    bool recorded = false;
    std::vector<GraphNeighbor> dependencies;
    std::vector<GraphNeighbor> readers;
    std::size_t dependency_start = 0;
    std::size_t reader_start = 0;
    std::string expression;
    std::optional<exs::CompositionGraph> composition;

    void reset(const ViewerModel& model);
};

void draw_graph_view(ViewerModel& model, GraphViewState& state);

} // namespace snt::view

#endif
