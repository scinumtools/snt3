#include "data_page.h"

#include <snt/val/value_base.h>

#include <algorithm>
#include <stdexcept>

namespace snt::view {
namespace {

std::string scalar_text(val::BaseValue& page, const std::vector<std::size_t>& coordinates) {
    if (page.get_shape().empty()) return page.to_string();
    val::Array::RangeType ranges;
    ranges.reserve(coordinates.size());
    for (const auto index : coordinates) ranges.push_back({index, index});
    return page.slice(ranges)->to_string();
}

} // namespace

DataCells read_array_page(const dip::Environment& environment, const std::string& path,
                          const val::Array::ShapeType& shape, const std::vector<std::size_t>& fixed_indices,
                          std::size_t row_axis, std::size_t column_axis, std::size_t row_start,
                          std::size_t column_start, std::size_t row_limit, std::size_t column_limit) {
    if (shape.empty() || fixed_indices.size() != shape.size() || row_axis >= shape.size() ||
        (shape.size() > 1 && (column_axis >= shape.size() || row_axis == column_axis)) ||
        row_limit == 0 || column_limit == 0)
        throw std::invalid_argument("Invalid array page axes or limits");
    if (std::any_of(shape.begin(), shape.end(), [](std::size_t extent) { return extent == 0; }) ||
        row_start >= shape[row_axis] ||
        (shape.size() > 1 && column_start >= shape[column_axis]))
        throw std::out_of_range("Array page starts outside the value");
    const bool matrix = shape.size() > 1;
    const auto rows = std::min(row_limit, shape[row_axis] - row_start);
    const auto columns = matrix ? std::min(column_limit, shape[column_axis] - column_start) : 1;
    const auto row_end = row_start + rows - 1;
    const auto column_end = matrix ? column_start + columns - 1 : 0;
    val::Array::RangeType ranges;
    ranges.reserve(shape.size());
    for (std::size_t axis = 0; axis < shape.size(); ++axis) {
        if (axis == row_axis) ranges.push_back({row_start, row_end});
        else if (matrix && axis == column_axis) ranges.push_back({column_start, column_end});
        else {
            if (fixed_indices[axis] >= shape[axis])
                throw std::out_of_range("Fixed array index outside the value");
            ranges.push_back({fixed_indices[axis], fixed_indices[axis]});
        }
    }
    auto page = dip::read_value_slice(environment, path, ranges);
    DataCells cells(rows, std::vector<std::string>(columns));
    std::vector<std::size_t> retained_axes;
    for (std::size_t axis = 0; axis < shape.size(); ++axis)
        if (ranges[axis].dmax > ranges[axis].dmin) retained_axes.push_back(axis);
    std::vector<std::size_t> coordinates(retained_axes.size(), 0);
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t axis = 0; axis < retained_axes.size(); ++axis)
            if (retained_axes[axis] == row_axis) coordinates[axis] = row;
        for (std::size_t column = 0; column < columns; ++column) {
            if (matrix)
                for (std::size_t axis = 0; axis < retained_axes.size(); ++axis)
                    if (retained_axes[axis] == column_axis) coordinates[axis] = column;
            cells[row][column] = scalar_text(*page, coordinates);
        }
    }
    return cells;
}

DataCells read_table_page(const dip::Environment& environment, const dip::TableInspection& table,
                          std::size_t row_start, std::size_t column_start,
                          std::size_t row_limit, std::size_t column_limit) {
    if (row_limit == 0 || column_limit == 0 || row_start >= table.rows ||
        column_start >= table.columns.size())
        throw std::invalid_argument("Invalid table page range");
    const auto rows = std::min(row_limit, table.rows - row_start);
    const auto columns = std::min(column_limit, table.columns.size() - column_start);
    DataCells cells(rows, std::vector<std::string>(columns));
    for (std::size_t column = 0; column < columns; ++column) {
        const auto& info = table.columns[column_start + column];
        auto page = dip::read_value_slice(environment, info.path, {{row_start, row_start + rows - 1}});
        for (std::size_t row = 0; row < rows; ++row)
            cells[row][column] = scalar_text(*page, {row});
    }
    return cells;
}

} // namespace snt::view
