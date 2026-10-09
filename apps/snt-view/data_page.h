#ifndef SNT_VIEW_DATA_PAGE_H
#define SNT_VIEW_DATA_PAGE_H

#include <snt/dip/inspect/inspector.h>

#include <cstddef>
#include <string>
#include <vector>

namespace snt::view {

using DataCells = std::vector<std::vector<std::string>>;

DataCells read_array_page(const dip::Environment& environment, const std::string& path,
                          const val::Array::ShapeType& shape, const std::vector<std::size_t>& fixed_indices,
                          std::size_t row_axis, std::size_t column_axis, std::size_t row_start,
                          std::size_t column_start, std::size_t row_limit, std::size_t column_limit);

DataCells read_table_page(const dip::Environment& environment, const dip::TableInspection& table,
                          std::size_t row_start, std::size_t column_start,
                          std::size_t row_limit, std::size_t column_limit);

} // namespace snt::view

#endif
