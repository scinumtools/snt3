#ifndef SNT_VIEW_DATA_PAGE_H
#define SNT_VIEW_DATA_PAGE_H

#include <cstddef>
#include <snt/dip/inspect/inspector.h>
#include <string>
#include <vector>

namespace snt::view {

    using DataCells = std::vector<std::vector<std::string>>; ///< Row-major display text for a bounded data page.

    /** Read a bounded rectangular array slice. Other axes use fixed_indices; invalid axes or ranges throw. */
    DataCells read_array_page(
        const dip::Environment& environment,
        const std::string& path,
        const val::Array::ShapeType& shape,
        const std::vector<std::size_t>& fixed_indices,
        std::size_t row_axis,
        std::size_t column_axis,
        std::size_t row_start,
        std::size_t column_start,
        std::size_t row_limit,
        std::size_t column_limit
    );

    /** Read a bounded table page as cell text; invalid starting coordinates or zero limits throw. */
    DataCells read_table_page(
        const dip::Environment& environment,
        const dip::TableInspection& table,
        std::size_t row_start,
        std::size_t column_start,
        std::size_t row_limit,
        std::size_t column_limit
    );

} // namespace snt::view

#endif
