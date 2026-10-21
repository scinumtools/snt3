#ifndef SNT_VIEW_DATA_VIEW_H
#define SNT_VIEW_DATA_VIEW_H

#include "viewer_model.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace snt::view {

    /** Cache the selected value and visible array or table page for the current model revision. */
    struct DataViewState {
        std::string path;                            ///< Selected browser object path.
        std::string value_path;                      ///< Underlying value path used for slicing.
        std::size_t revision = 0;                    ///< Model revision represented by this cache.
        std::optional<dip::ValueSummary> summary;    ///< Selected value metadata when available.
        std::optional<dip::TableInspection> table;   ///< Table column metadata when the selection is a table.
        std::vector<std::size_t> fixed_indices;      ///< Indices for array axes outside the displayed plane.
        std::size_t row_axis = 0;                    ///< Array axis displayed vertically.
        std::size_t column_axis = 0;                 ///< Array axis displayed horizontally.
        std::size_t row_start = 0;                   ///< First visible row, zero-based.
        std::size_t column_start = 0;                ///< First visible column, zero-based.
        std::vector<std::vector<std::string>> cells; ///< Cached row-major display text.
        bool dirty = true;                           ///< Whether the page must be loaded again.
        std::string error;                           ///< Most recent data access failure for display.

        /** Refresh selection metadata and invalidate the cached page for the model. */
        void reset(const ViewerModel& model);
    };

    /** Draw data controls and the current bounded page, loading cells when dirty. */
    void draw_data_view(ViewerModel& model, DataViewState& state);

} // namespace snt::view

#endif
