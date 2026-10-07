#include "graph_view.h"

#include <snt/dip/inspect/dependency_graph.h>

#include <imgui.h>

#include <algorithm>
#include <string>
#include <unordered_set>

namespace snt::view {
namespace {

constexpr std::size_t visible_neighbors = 12;
constexpr float node_width = 170.0f;
constexpr float node_height = 32.0f;
constexpr float column_gap = 55.0f;
constexpr float row_step = 48.0f;

std::string object_path(const std::string& id) {
    return !id.empty() && id.front() == '?' ? id.substr(1) : id;
}

std::string short_label(const std::string& id) {
    const std::string path = object_path(id);
    if (path.size() <= 25) return path;
    return "..." + path.substr(path.size() - 22);
}

void draw_node(ViewerModel& model, const std::string& id, const std::string& request,
               const std::string& operand, ImVec2 position, bool selected) {
    const std::string target = object_path(id);
    const bool navigable = model.object(target) != nullptr;
    ImGui::SetCursorScreenPos(position);
    ImGui::PushID(id.c_str());
    ImGui::InvisibleButton("node", ImVec2(node_width, node_height));
    const bool hovered = ImGui::IsItemHovered();
    if (hovered) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(id.c_str());
        if (!request.empty()) ImGui::Text("Request: %s", request.c_str());
        if (!operand.empty()) ImGui::Text("Operand: %s", operand.c_str());
        if (!navigable) ImGui::TextDisabled("Not available in this browser");
        ImGui::EndTooltip();
    }
    if (navigable && ImGui::IsItemClicked()) model.select(target);
    ImGui::PopID();

    auto* draw = ImGui::GetWindowDrawList();
    const auto background = ImGui::GetColorU32(selected ? ImGuiCol_HeaderActive
                                            : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_FrameBg);
    draw->AddRectFilled(position, ImVec2(position.x + node_width, position.y + node_height), background, 5.0f);
    draw->AddRect(position, ImVec2(position.x + node_width, position.y + node_height),
                  ImGui::GetColorU32(ImGuiCol_Border), 5.0f);
    const auto label = short_label(id);
    draw->PushClipRect(ImVec2(position.x + 7, position.y),
                       ImVec2(position.x + node_width - 7, position.y + node_height), true);
    draw->AddText(ImVec2(position.x + 8, position.y + (node_height - ImGui::GetFontSize()) / 2),
                  ImGui::GetColorU32(navigable ? ImGuiCol_Text : ImGuiCol_TextDisabled), label.c_str());
    draw->PopClipRect();
}

void draw_arrow(ImVec2 from, ImVec2 to) {
    auto* draw = ImGui::GetWindowDrawList();
    const auto color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    draw->AddLine(from, to, color, 1.5f);
    draw->AddTriangleFilled(to, ImVec2(to.x - 8, to.y - 4), ImVec2(to.x - 8, to.y + 4), color);
}

void draw_composition_node(const exs::CompositionGraph& graph, std::size_t index, std::size_t depth) {
    if (index >= graph.nodes.size() || depth > graph.nodes.size()) return;
    const auto& node = graph.nodes[index];
    ImGui::PushID(static_cast<int>(index));
    const auto flags = node.children.empty() ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen : 0;
    const bool open = ImGui::TreeNodeEx("composition", flags, "%s", node.text.c_str());
    if (open && !node.children.empty()) {
        for (const auto child : node.children) draw_composition_node(graph, child, depth + 1);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

} // namespace

void GraphViewState::reset(const ViewerModel& model) {
    *this = {};
    path = model.selection();
    revision = model.revision();
    const auto* object = model.object(path);
    if (!object) return;
    const auto& graph = model.environment().dependency_graph();
    recorded = graph.recorded;
    if (!recorded) return;
    const std::string id = object->source_name.empty() ? "?" + object->node_path : object->path;
    const auto* value_event = graph.latest(id, dip::DependencyEventKind::Value);
    const auto* condition_event = graph.latest(id, dip::DependencyEventKind::Condition);
    const auto* event = value_event && (!value_event->reads.empty() || value_event->composition)
        ? value_event : condition_event;
    if (event) {
        expression = event->expression;
        composition = event->composition;
    }
    std::unordered_set<std::string> seen;
    if (value_event)
        for (const auto& edge : value_event->reads)
            if (seen.insert(edge.target).second)
                dependencies.push_back({edge.target, edge.request, edge.operand});
    if (condition_event)
        for (const auto& edge : condition_event->reads)
            if (seen.insert(edge.target).second)
                dependencies.push_back({edge.target, edge.request, edge.operand});
    for (const auto& reader : graph.referenced_by(id)) {
        GraphNeighbor neighbor{reader, {}, {}};
        for (const auto& edge : graph.dependencies(reader)) {
            if (edge.target == id) {
                neighbor.request = edge.request;
                neighbor.operand = edge.operand;
                break;
            }
        }
        readers.push_back(std::move(neighbor));
    }
}

void draw_graph_view(ViewerModel& model, GraphViewState& state) {
    if (state.path != model.selection() || state.revision != model.revision()) state.reset(model);
    if (!state.recorded) {
        ImGui::TextDisabled("Dependency recording is unavailable for this artifact.");
        return;
    }
    const auto* object = model.object(state.path);
    if (!object) return;
    const std::string center_id = object->source_name.empty() ? "?" + object->node_path : object->path;
    ImGui::TextDisabled("Dependencies -> selected value -> referenced by");
    if (state.dependencies.size() > visible_neighbors) {
        ImGui::BeginDisabled(state.dependency_start == 0);
        if (ImGui::SmallButton("Previous dependencies"))
            state.dependency_start = state.dependency_start > visible_neighbors
                ? state.dependency_start - visible_neighbors : 0;
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(state.dependency_start + visible_neighbors >= state.dependencies.size());
        if (ImGui::SmallButton("Next dependencies"))
            state.dependency_start += visible_neighbors;
        ImGui::EndDisabled();
    }
    if (state.readers.size() > visible_neighbors) {
        ImGui::BeginDisabled(state.reader_start == 0);
        if (ImGui::SmallButton("Previous readers"))
            state.reader_start = state.reader_start > visible_neighbors
                ? state.reader_start - visible_neighbors : 0;
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(state.reader_start + visible_neighbors >= state.readers.size());
        if (ImGui::SmallButton("Next readers")) state.reader_start += visible_neighbors;
        ImGui::EndDisabled();
    }
    const auto left_count = std::min(state.dependencies.size() - state.dependency_start, visible_neighbors);
    const auto right_count = std::min(state.readers.size() - state.reader_start, visible_neighbors);
    const float height = std::max(150.0f, 24.0f + row_step * static_cast<float>(
        std::max(left_count, right_count)));
    const float width = 3 * node_width + 2 * column_gap + 24;
    ImGui::BeginChild("Local dependency graph", ImVec2(0, std::min(height + 12, 470.0f)),
                      ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float center_y = origin.y + (height - node_height) / 2;
    const float center_x = origin.x + node_width + column_gap + 12;
    const float right_x = center_x + node_width + column_gap;
    const auto node_y = [&](std::size_t index, std::size_t count) {
        return origin.y + (height - static_cast<float>(count) * row_step) / 2 +
               static_cast<float>(index) * row_step + 8;
    };
    for (std::size_t i = 0; i < left_count; ++i)
        draw_arrow(ImVec2(origin.x + node_width + 12, node_y(i, left_count) + node_height / 2),
                   ImVec2(center_x, center_y + node_height / 2));
    for (std::size_t i = 0; i < right_count; ++i)
        draw_arrow(ImVec2(center_x + node_width, center_y + node_height / 2),
                   ImVec2(right_x, node_y(i, right_count) + node_height / 2));
    ImGui::PushID("dependencies");
    for (std::size_t i = 0; i < left_count; ++i) {
        ImGui::PushID(static_cast<int>(state.dependency_start + i));
        const auto& neighbor = state.dependencies[state.dependency_start + i];
        draw_node(model, neighbor.id, neighbor.request, neighbor.operand,
                  ImVec2(origin.x + 12, node_y(i, left_count)), false);
        ImGui::PopID();
    }
    ImGui::PopID();
    ImGui::PushID("readers");
    for (std::size_t i = 0; i < right_count; ++i) {
        ImGui::PushID(static_cast<int>(state.reader_start + i));
        const auto& neighbor = state.readers[state.reader_start + i];
        draw_node(model, neighbor.id, neighbor.request, neighbor.operand,
                  ImVec2(right_x, node_y(i, right_count)), false);
        ImGui::PopID();
    }
    ImGui::PopID();
    ImGui::PushID("selected");
    draw_node(model, center_id, {}, {}, ImVec2(center_x, center_y), true);
    ImGui::PopID();
    ImGui::SetCursorScreenPos(origin);
    ImGui::Dummy(ImVec2(width, height));
    ImGui::EndChild();
    if (state.dependencies.size() > left_count || state.readers.size() > right_count)
        ImGui::TextDisabled("Dependencies %zu to %zu of %zu; readers %zu to %zu of %zu.",
                            state.dependency_start + (left_count ? 1 : 0), state.dependency_start + left_count,
                            state.dependencies.size(), state.reader_start + (right_count ? 1 : 0),
                            state.reader_start + right_count, state.readers.size());
    if (!state.expression.empty() || state.composition) {
        ImGui::SeparatorText("Expression");
        if (!state.expression.empty()) ImGui::TextWrapped("%s", state.expression.c_str());
        if (state.composition && state.composition->root < state.composition->nodes.size()) {
            ImGui::SeparatorText("Composition");
            draw_composition_node(*state.composition, state.composition->root, 0);
        }
    }
}

} // namespace snt::view
