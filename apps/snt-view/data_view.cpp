#include "data_view.h"
#include "data_page.h"

#include <snt/core/datatypes.h>

#include <imgui.h>

#include <algorithm>
#include <string>

namespace snt::view {
namespace {

constexpr std::size_t array_rows = 24;
constexpr std::size_t array_columns = 8;
constexpr std::size_t table_rows = 32;
constexpr std::size_t table_columns = 8;
constexpr float min_column_width = 72.0f;
constexpr float max_column_width = 280.0f;

float fitted_column_width(const std::string& heading, const DataCells& cells, std::size_t column) {
    float width = ImGui::CalcTextSize(heading.c_str()).x;
    for (const auto& row : cells)
        width = std::max(width, ImGui::CalcTextSize(row[column].c_str()).x);
    return std::clamp(width + 2.0f * ImGui::GetStyle().CellPadding.x + 12.0f,
                      min_column_width, max_column_width);
}

void data_cell(const std::string& value) {
    const float available = ImGui::GetContentRegionAvail().x;
    ImGui::TextUnformatted(value.c_str());
    if (ImGui::IsItemHovered() && ImGui::CalcTextSize(value.c_str()).x > available) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 45.0f);
        ImGui::TextUnformatted(value.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

std::string table_heading(const dip::TableColumnInspection& column) {
    return column.name + (column.units
        ? " [" + column.units->measurement.baseunits.to_string() + "]" : "");
}

void page_buttons(const char* label, std::size_t& start, std::size_t total,
                  std::size_t page_size, bool one_based, bool& dirty) {
    if (total == 0) return;
    const auto end = start + std::min(page_size, total - start);
    if (one_based && total <= page_size)
        ImGui::Text("Showing all %zu %s", total, label);
    else
        ImGui::Text("Showing %s %zu to %zu of %zu", label, start + (one_based ? 1 : 0),
                    end - (one_based ? 0 : 1), total);
    if (total <= page_size) return;
    ImGui::SameLine();
    ImGui::BeginDisabled(start == 0);
    if (ImGui::SmallButton((std::string("Previous##") + label).c_str())) {
        start = start > page_size ? start - page_size : 0;
        dirty = true;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(end >= total);
    if (ImGui::SmallButton((std::string("Next##") + label).c_str())) {
        start = end;
        dirty = true;
    }
    ImGui::EndDisabled();
}

float data_table_height(std::size_t rows) {
    const auto& style = ImGui::GetStyle();
    const float row_height = ImGui::GetTextLineHeight() + 2.0f * style.CellPadding.y;
    const float content_height = static_cast<float>(rows + 1) * row_height + 2.0f;
    return std::min(content_height, std::max(row_height, ImGui::GetContentRegionAvail().y));
}

void draw_array(ViewerModel& model, DataViewState& state) {
    const auto& summary = *state.summary;
    const auto& shape = summary.shape;
    const bool matrix = shape.size() > 1;
    const auto type = core::DataTypeNames.find(summary.type);
    ImGui::Text("%s, %zu elements", type == core::DataTypeNames.end() ? "Array" : type->second.c_str(),
                summary.elements);
    ImGui::SameLine();
    if (summary.units) ImGui::TextDisabled("[%s]", summary.units->measurement.baseunits.to_string().c_str());
    ImGui::TextUnformatted("Shape:");
    ImGui::SameLine();
    for (std::size_t axis = 0; axis < shape.size(); ++axis) {
        if (axis) ImGui::SameLine();
        ImGui::Text("%s%zu", axis ? "x " : "", shape[axis]);
    }
    if (std::any_of(shape.begin(), shape.end(), [](std::size_t extent) { return extent == 0; })) {
        ImGui::TextDisabled("This array is empty.");
        return;
    }

    if (matrix) {
        const auto choose_axis = [&](const char* label, std::size_t& chosen, std::size_t other) {
            const std::string preview = "Dimension " + std::to_string(chosen);
            if (ImGui::BeginCombo(label, preview.c_str())) {
                for (std::size_t axis = 0; axis < shape.size(); ++axis) {
                    if (axis == other) continue;
                    const std::string name = "Dimension " + std::to_string(axis);
                    if (ImGui::Selectable(name.c_str(), axis == chosen)) {
                        chosen = axis;
                        state.row_start = 0;
                        state.column_start = 0;
                        state.dirty = true;
                    }
                }
                ImGui::EndCombo();
            }
        };
        choose_axis("Rows", state.row_axis, state.column_axis);
        choose_axis("Columns", state.column_axis, state.row_axis);
    }
    for (std::size_t axis = 0; axis < shape.size(); ++axis) {
        if (axis == state.row_axis || (matrix && axis == state.column_axis)) continue;
        const std::string label = "Dimension " + std::to_string(axis) + " index";
        std::size_t index = state.fixed_indices[axis];
        if (ImGui::InputScalar(label.c_str(), ImGuiDataType_U64, &index)) {
            state.fixed_indices[axis] = std::min(index, shape[axis] - 1);
            state.dirty = true;
        }
    }
    page_buttons(matrix ? "row indices" : "indices", state.row_start, shape[state.row_axis],
                 array_rows, false, state.dirty);
    if (matrix)
        page_buttons("column indices", state.column_start, shape[state.column_axis],
                     array_columns, false, state.dirty);
    if (state.dirty) {
        state.cells = read_array_page(model.environment(), state.value_path, shape, state.fixed_indices,
                                      state.row_axis, state.column_axis, state.row_start, state.column_start,
                                      array_rows, array_columns);
        state.dirty = false;
    }

    const auto visible_columns = state.cells.empty() ? 0 : state.cells.front().size();
    ImGui::PushID(state.path.c_str());
    const auto page_id = std::to_string(state.column_start);
    ImGui::PushID(page_id.c_str());
    if (ImGui::BeginTable("Array data", static_cast<int>(visible_columns + 1),
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                              ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                          ImVec2(0, data_table_height(state.cells.size())))) {
        ImGui::TableSetupColumn(matrix ? "Row" : "Index", ImGuiTableColumnFlags_WidthFixed, min_column_width);
        for (std::size_t column = 0; column < visible_columns; ++column) {
            const std::string heading = matrix ? std::to_string(state.column_start + column) : "Value";
            ImGui::TableSetupColumn(heading.c_str(), ImGuiTableColumnFlags_WidthFixed,
                                    fitted_column_width(heading, state.cells, column));
        }
        ImGui::TableHeadersRow();
        for (std::size_t row = 0; row < state.cells.size(); ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%zu", state.row_start + row);
            for (std::size_t column = 0; column < visible_columns; ++column) {
                ImGui::TableSetColumnIndex(static_cast<int>(column + 1));
                data_cell(state.cells[row][column]);
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopID();
    ImGui::PopID();
}

void draw_table(ViewerModel& model, DataViewState& state) {
    const auto& table = *state.table;
    ImGui::Text("%zu rows, %zu columns", table.rows, table.columns.size());
    if (table.rows == 0) {
        ImGui::TextDisabled("This table has no rows.");
        return;
    }
    page_buttons("rows", state.row_start, table.rows, table_rows, true, state.dirty);
    page_buttons("columns", state.column_start, table.columns.size(), table_columns, true, state.dirty);
    if (state.dirty) {
        state.cells = read_table_page(model.environment(), table, state.row_start, state.column_start,
                                      table_rows, table_columns);
        state.dirty = false;
    }
    const auto visible_columns = state.cells.empty()
        ? std::min(table_columns, table.columns.size() - state.column_start) : state.cells.front().size();
    ImGui::PushID(state.path.c_str());
    const auto page_id = std::to_string(state.column_start);
    ImGui::PushID(page_id.c_str());
    if (ImGui::BeginTable("Table data", static_cast<int>(visible_columns + 1),
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                              ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                          ImVec2(0, data_table_height(state.cells.size())))) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, min_column_width);
        for (std::size_t column = 0; column < visible_columns; ++column) {
            const auto& info = table.columns[state.column_start + column];
            const std::string heading = table_heading(info);
            ImGui::TableSetupColumn(heading.c_str(), ImGuiTableColumnFlags_WidthFixed,
                                    fitted_column_width(heading, state.cells, column));
        }
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("#");
        for (std::size_t column = 0; column < visible_columns; ++column) {
            const auto& info = table.columns[state.column_start + column];
            ImGui::TableSetColumnIndex(static_cast<int>(column + 1));
            const std::string heading = table_heading(info);
            ImGui::PushID(static_cast<int>(column));
            if (ImGui::Selectable(heading.c_str())) model.select(info.path);
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(info.path.c_str());
                if (!info.metadata.description.empty())
                    ImGui::TextWrapped("%s", info.metadata.description.c_str());
                ImGui::EndTooltip();
            }
            ImGui::PopID();
        }
        for (std::size_t row = 0; row < state.cells.size(); ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%zu", state.row_start + row + 1);
            for (std::size_t column = 0; column < visible_columns; ++column) {
                ImGui::TableSetColumnIndex(static_cast<int>(column + 1));
                data_cell(state.cells[row][column]);
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopID();
    ImGui::PopID();
}

} // namespace

void DataViewState::reset(const ViewerModel& model) {
    *this = {};
    path = model.selection();
    revision = model.revision();
    const auto* object = model.object(path);
    if (!object) return;
    value_path = object->source_name.empty() ? object->node_path : object->path;
    try {
        const auto capabilities = dip::inspect_capabilities(model.environment(), value_path);
        if (capabilities.hasTabularData) {
            table = dip::inspect_table(model.environment(), object->node_path);
        } else if (capabilities.hasArrayData) {
            summary = dip::inspect_value_summary(model.environment(), value_path);
            const auto& shape = summary->shape;
            fixed_indices.resize(shape.size(), 0);
            row_axis = shape.size() > 1 ? shape.size() - 2 : 0;
            column_axis = shape.empty() ? 0 : shape.size() - 1;
        }
    } catch (const std::exception& problem) {
        error = problem.what();
    }
}

void draw_data_view(ViewerModel& model, DataViewState& state) {
    if (state.path != model.selection() || state.revision != model.revision()) state.reset(model);
    if (!state.error.empty()) {
        ImGui::TextWrapped("Data unavailable: %s", state.error.c_str());
        return;
    }
    try {
        if (state.table) draw_table(model, state);
        else if (state.summary && !state.summary->shape.empty()) draw_array(model, state);
        else ImGui::TextDisabled("No array or table data for this selection.");
    } catch (const std::exception& problem) {
        state.error = problem.what();
        ImGui::TextWrapped("Data unavailable: %s", state.error.c_str());
    }
}

} // namespace snt::view
