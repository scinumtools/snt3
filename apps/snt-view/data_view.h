#ifndef SNT_VIEW_DATA_VIEW_H
#define SNT_VIEW_DATA_VIEW_H

#include "viewer_model.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace snt::view {

struct DataViewState {
    std::string path;
    std::string value_path;
    std::size_t revision = 0;
    std::optional<dip::ValueSummary> summary;
    std::optional<dip::TableInspection> table;
    std::vector<std::size_t> fixed_indices;
    std::size_t row_axis = 0;
    std::size_t column_axis = 0;
    std::size_t row_start = 0;
    std::size_t column_start = 0;
    std::vector<std::vector<std::string>> cells;
    bool dirty = true;
    std::string error;

    void reset(const ViewerModel& model);
};

void draw_data_view(ViewerModel& model, DataViewState& state);

} // namespace snt::view

#endif
