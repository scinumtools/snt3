#include "browser_view.h"

#include <imgui.h>

#include <algorithm>
#include <string>
#include <unordered_set>

namespace snt::view {

const char* object_kind_name(const ObjectInfo& object) {
    if (object.role == ObjectRole::Project) return "Resolved nodes";
    if (object.role == ObjectRole::DIPfile) return "DIPfile manifest";
    if (object.role == ObjectRole::ManifestCategory) return "DIPfile category";
    if (object.role == ObjectRole::ManifestEntry) return "DIPfile entry";
    if (object.role == ObjectRole::LocalSources) return "Local sources";
    if (object.role == ObjectRole::BlockSources) return "Block value sources";
    if (object.role == ObjectRole::Sources) return "Named sources";
    if (object.role == ObjectRole::RawSources) return "Raw named sources";
    if (object.role == ObjectRole::Source) return "Named source";
    if (object.role == ObjectRole::CodeSource) return "Local source";
    if (object.role == ObjectRole::BlockSource) return "Block value source";
    if (object.role == ObjectRole::Overrides) return "Overridden nodes";
    if (object.role == ObjectRole::Override) return "Overridden node";
    if (object.role == ObjectRole::Schemas) return "Schemas";
    if (object.role == ObjectRole::Schema) return "Schema";
    if (object.role == ObjectRole::Units) return "Custom units";
    if (object.role == ObjectRole::Unit) return "Custom unit";
    if (object.has_table) return "Table";
    if (object.has_value && !object.children.empty()) return "Value and children";
    if (object.has_value) return "Value";
    if (object.hierarchy_kind == dip::Path::Kind::Map) return "Map collection";
    if (object.hierarchy_kind == dip::Path::Kind::List) return "List collection";
    if (object.hierarchy_kind == dip::Path::Kind::Item) return "Collection item";
    return "Group";
}

namespace {

bool matches(const ViewerModel& model, const ObjectInfo& object, const std::string& query) {
    if (query.empty() || object.path.find(query) != std::string::npos ||
        object.label.find(query) != std::string::npos) return true;
    for (const auto& child : object.children)
        if (const auto* item = model.object(child); item && matches(model, *item, query)) return true;
    return false;
}

void draw_kind_icon(const ObjectInfo& object, ImVec2 pos, float size) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
    const auto point = [&](float x, float y) { return ImVec2(pos.x + x * size, pos.y + y * size); };
    if (object.role == ObjectRole::DIPfile || object.role == ObjectRole::ManifestCategory ||
        object.role == ObjectRole::ManifestEntry) {
        for (float x : {0.27f, 0.73f})
            for (float y : {0.27f, 0.73f})
                draw->AddCircle(point(x, y), size * 0.17f, color, 12, 1.2f);
        for (float x : {0.40f, 0.60f})
            draw->AddLine(point(x, 0.27f), point(x, 0.73f), color, 1.2f);
        for (float y : {0.40f, 0.60f})
            draw->AddLine(point(0.27f, y), point(0.73f, y), color, 1.2f);
    } else if (object.role == ObjectRole::Schemas || object.role == ObjectRole::Schema) {
        draw->PathLineTo(point(0.06f, 0.36f));
        draw->PathLineTo(point(0.23f, 0.24f));
        draw->PathLineTo(point(0.39f, 0.28f));
        draw->PathLineTo(point(0.5f, 0.42f));
        draw->PathLineTo(point(0.61f, 0.28f));
        draw->PathLineTo(point(0.77f, 0.24f));
        draw->PathLineTo(point(0.94f, 0.36f));
        draw->PathLineTo(point(0.8f, 0.72f));
        draw->PathLineTo(point(0.61f, 0.62f));
        draw->PathLineTo(point(0.5f, 0.86f));
        draw->PathLineTo(point(0.39f, 0.62f));
        draw->PathLineTo(point(0.2f, 0.72f));
        draw->PathStroke(color, 1.2f, ImDrawFlags_Closed);
        draw->AddCircleFilled(point(0.31f, 0.46f), size * 0.085f, color, 6);
        draw->AddCircleFilled(point(0.69f, 0.46f), size * 0.085f, color, 6);
    } else if (object.role == ObjectRole::LocalSources || object.role == ObjectRole::CodeSource ||
               object.role == ObjectRole::BlockSources || object.role == ObjectRole::BlockSource ||
               object.role == ObjectRole::Sources || object.role == ObjectRole::RawSources ||
               object.role == ObjectRole::Source) {
        draw->AddLine(point(0.29f, 0.24f), point(0.06f, 0.50f), color, 1.2f);
        draw->AddLine(point(0.06f, 0.50f), point(0.29f, 0.76f), color, 1.2f);
        draw->AddLine(point(0.59f, 0.15f), point(0.40f, 0.85f), color, 1.2f);
        draw->AddLine(point(0.71f, 0.24f), point(0.94f, 0.50f), color, 1.2f);
        draw->AddLine(point(0.94f, 0.50f), point(0.71f, 0.76f), color, 1.2f);
    } else if (object.role == ObjectRole::Units || object.role == ObjectRole::Unit) {
        draw->AddRect(point(0.08f, 0.28f), point(0.92f, 0.76f), color);
        for (int tick = 0; tick < 4; ++tick) {
            const float x = 0.24f + tick * 0.17f;
            draw->AddLine(point(x, 0.28f), point(x, tick % 2 ? 0.48f : 0.57f), color);
        }
    } else if (object.has_table) {
        draw->AddRect(point(0.08f, 0.12f), point(0.92f, 0.88f), color, 1.2f);
        draw->AddLine(point(0.08f, 0.36f), point(0.92f, 0.36f), color, 1.2f);
        draw->AddLine(point(0.08f, 0.62f), point(0.92f, 0.62f), color, 1.2f);
        draw->AddLine(point(0.49f, 0.12f), point(0.49f, 0.88f), color, 1.2f);
    } else if (object.has_value && object.children.empty()) {
        draw->AddQuadFilled(point(0.5f, 0.06f), point(0.94f, 0.5f),
                            point(0.5f, 0.94f), point(0.06f, 0.5f), color);
    } else if (!object.has_value && object.hierarchy_kind == dip::Path::Kind::Map) {
        for (int row = 0; row < 2; ++row)
            for (int column = 0; column < 2; ++column)
                draw->AddRect(point(0.08f + column * 0.47f, 0.08f + row * 0.47f),
                              point(0.45f + column * 0.47f, 0.45f + row * 0.47f), color);
    } else if (!object.has_value && object.hierarchy_kind == dip::Path::Kind::List) {
        for (int row = 0; row < 3; ++row) {
            const float y = 0.2f + row * 0.3f;
            draw->AddCircleFilled(point(0.15f, y), size * 0.055f, color);
            draw->AddLine(point(0.35f, y), point(0.92f, y), color);
        }
    } else {
        draw->AddLine(point(0.08f, 0.25f), point(0.08f, 0.39f), color);
        draw->AddLine(point(0.08f, 0.25f), point(0.39f, 0.25f), color);
        draw->AddLine(point(0.39f, 0.25f), point(0.5f, 0.39f), color);
        draw->AddLine(point(0.5f, 0.39f), point(0.92f, 0.39f), color);
        draw->AddRect(point(0.08f, 0.39f), point(0.92f, 0.88f), color, size * 0.1f);
        if (object.has_value)
            draw->AddCircleFilled(point(0.76f, 0.69f), size * 0.13f, color);
    }
}

struct TreeExpansionState {
    std::unordered_set<std::string> pending;
    bool open = true;
    bool suppress_selection_open = false;
    std::string selection_at_collapse;
};

void mark_tree_nodes(const ViewerModel& model, const ObjectInfo& object,
                     std::unordered_set<std::string>& paths) {
    for (const auto& path : object.children) {
        const auto* item = model.object(path);
        if (!item || item->children.empty()) continue;
        paths.insert(path);
        mark_tree_nodes(model, *item, paths);
    }
}

bool draw_expansion_button(const char* id, bool expand) {
    const float size = ImGui::GetFrameHeight();
    const bool clicked = ImGui::Button(id, ImVec2(size, size));
    const ImVec2 pos = ImGui::GetItemRectMin();
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
    auto* draw = ImGui::GetWindowDrawList();
    const auto point = [&](float x, float y) { return ImVec2(pos.x + x * size, pos.y + y * size); };
    draw->AddRect(point(0.2f, 0.2f), point(0.8f, 0.8f), color);
    draw->AddLine(point(0.34f, 0.5f), point(0.66f, 0.5f), color);
    if (expand) draw->AddLine(point(0.5f, 0.34f), point(0.5f, 0.66f), color);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s all branches", expand ? "Expand" : "Collapse");
    return clicked;
}

bool draw_history_button(const char* id, bool forward, bool enabled) {
    const float size = ImGui::GetFrameHeight();
    ImGui::BeginDisabled(!enabled);
    const bool clicked = ImGui::Button(id, ImVec2(size, size));
    const ImVec2 pos = ImGui::GetItemRectMin();
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
    auto* draw = ImGui::GetWindowDrawList();
    const auto point = [&](float x, float y) { return ImVec2(pos.x + x * size, pos.y + y * size); };
    const float tip = forward ? 0.74f : 0.26f;
    const float tail = forward ? 0.26f : 0.74f;
    const float shoulder = forward ? 0.54f : 0.46f;
    draw->AddLine(point(tail, 0.5f), point(tip, 0.5f), color, 1.5f);
    draw->AddLine(point(shoulder, 0.3f), point(tip, 0.5f), color, 1.5f);
    draw->AddLine(point(shoulder, 0.7f), point(tip, 0.5f), color, 1.5f);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s in browsing history", forward ? "Forward" : "Back");
    ImGui::EndDisabled();
    return clicked;
}

void draw_tree(ViewerModel& model, const ObjectInfo& object, const std::string& query,
               TreeExpansionState& expansion) {
    for (const auto& path : object.children) {
        const auto* item = model.object(path);
        if (!item || !matches(model, *item, query)) continue;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                   ImGuiTreeNodeFlags_SpanAvailWidth;
        if (item->children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (model.selection() == path) flags |= ImGuiTreeNodeFlags_Selected;
        if (!query.empty()) flags |= ImGuiTreeNodeFlags_DefaultOpen;
        if (item->role == ObjectRole::Project || item->role == ObjectRole::DIPfile ||
            item->role == ObjectRole::LocalSources || item->role == ObjectRole::BlockSources ||
            item->role == ObjectRole::Sources || item->role == ObjectRole::RawSources ||
            item->role == ObjectRole::Overrides || item->role == ObjectRole::Schemas ||
            item->role == ObjectRole::Units)
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        if (expansion.pending.erase(path) > 0) {
            ImGui::SetNextItemOpen(expansion.open, ImGuiCond_Always);
        } else if (!expansion.suppress_selection_open) {
            const auto* selected = model.object(model.selection());
            for (std::string ancestor = selected ? selected->parent : std::string{};
                 !ancestor.empty();) {
                if (ancestor == path) {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                    break;
                }
                const auto* parent = model.object(ancestor);
                ancestor = parent ? parent->parent : std::string{};
            }
        }
        const ImVec2 cursor = ImGui::GetCursorScreenPos();
        const bool open = ImGui::TreeNodeEx(item->path.c_str(), flags, "   %s", item->label.c_str());
        const float icon_size = ImGui::GetFontSize() * 0.72f;
        const auto& style = ImGui::GetStyle();
        const ImVec2 icon_pos(cursor.x + ImGui::GetFontSize() + 2.0f * style.FramePadding.x + 1.0f,
                              ImGui::GetItemRectMin().y + (ImGui::GetItemRectSize().y - icon_size) * 0.5f);
        draw_kind_icon(*item, icon_pos, icon_size);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", object_kind_name(*item));
        if (ImGui::IsItemClicked()) model.select(path);
        if (open && !item->children.empty()) {
            draw_tree(model, *item, query, expansion);
            ImGui::TreePop();
        }
    }
}

} // namespace

void draw_browser(ViewerModel& model) {
    static char search[256] = {};
    static TreeExpansionState expansion;
    if (expansion.suppress_selection_open && model.selection() != expansion.selection_at_collapse)
        expansion.suppress_selection_open = false;
    ImGui::TextUnformatted("Browser");
    const float button_size = ImGui::GetFrameHeight();
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(std::max(1.0f, ImGui::GetContentRegionAvail().x -
                                   4.0f * (button_size + spacing)));
    if (ImGui::InputTextWithHint("##search", "Search paths", search, sizeof(search)))
        model.set_search(search);
    ImGui::SameLine();
    if (draw_history_button("##history_back", false, model.can_back())) model.back();
    ImGui::SameLine();
    if (draw_history_button("##history_forward", true, model.can_forward())) model.forward();
    ImGui::SameLine();
    const bool expand_all = draw_expansion_button("##expand_all", true);
    ImGui::SameLine();
    const bool collapse_all = draw_expansion_button("##collapse_all", false);
    if (const auto* root = model.object("")) {
        if (expand_all || collapse_all) {
            expansion.pending.clear();
            mark_tree_nodes(model, *root, expansion.pending);
            expansion.open = expand_all;
            expansion.suppress_selection_open = collapse_all;
            expansion.selection_at_collapse = model.selection();
        }
        draw_tree(model, *root, model.search(), expansion);
    }
}

} // namespace snt::view
