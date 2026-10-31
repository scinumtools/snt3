#include "source_graph_view.h"

#include "inspector_view.h"

#include <algorithm>
#include <imgui.h>
#include <optional>
#include <string>
#include <vector>

namespace snt::view {
    namespace {

        struct Card {
            float center_y = 0.0f;
        };

        std::string display_label(const ViewerModel& model, const ObjectInfo& object) {
            if (object.role == ObjectRole::ProjectFile && object.project_file_index &&
                *object.project_file_index < model.project_files().size())
                return model.project_files()[*object.project_file_index].path;
            return object.label;
        }

        Card draw_card(
            const ViewerModel& model,
            const ObjectInfo& object,
            const SourceTarget* target,
            bool selected,
            float width,
            std::optional<SourceTarget>& clicked_target,
            std::string& clicked_path
        ) {
            std::string label = display_label(model, object);
            if (target && target->line)
                label += ":" + std::to_string(target->line);
            ImGui::PushID(object.path.c_str());
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.04f, 0.5f));
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Header));
            const bool clicked = ImGui::Button(label.c_str(), ImVec2(width, 38.0f));
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(display_label(model, object).c_str());
                if (target) {
                    const auto file = model.display_file_path(target->file.string());
                    ImGui::Text("Open %s:%zu", file.c_str(), target->line);
                }
                ImGui::EndTooltip();
            }
            const float center_y = (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f;
            if (selected)
                ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopID();
            if (clicked) {
                if (target)
                    clicked_target = *target;
                else
                    clicked_path = object.path;
            }
            return {center_y};
        }

        void arrow(ImDrawList* draw, ImVec2 tip, ImU32 color) {
            draw->AddTriangleFilled(
                tip, ImVec2(tip.x - 5.0f, tip.y - 8.0f), ImVec2(tip.x + 5.0f, tip.y - 8.0f), color
            );
        }

        void draw_connections(
            const std::vector<Card>& dependencies,
            const Card& center,
            const std::vector<Card>& readers,
            float rail_x,
            float card_x,
            ImU32 color
        ) {
            auto* draw = ImGui::GetWindowDrawList();
            const float join_x = card_x - 10.0f;
            for (const auto& dependency : dependencies) {
                draw->AddLine(ImVec2(rail_x, dependency.center_y), ImVec2(join_x, dependency.center_y), color, 1.5f);
            }
            if (!dependencies.empty()) {
                draw->AddLine(ImVec2(rail_x, dependencies.front().center_y), ImVec2(rail_x, center.center_y), color, 1.5f);
                draw->AddLine(ImVec2(rail_x, center.center_y), ImVec2(join_x, center.center_y), color, 1.5f);
                arrow(draw, ImVec2(rail_x, center.center_y - 2.0f), color);
            }
            if (!readers.empty()) {
                draw->AddLine(ImVec2(rail_x, center.center_y), ImVec2(rail_x, readers.back().center_y), color, 1.5f);
                for (const auto& reader : readers) {
                    draw->AddLine(ImVec2(rail_x, reader.center_y), ImVec2(join_x, reader.center_y), color, 1.5f);
                    arrow(draw, ImVec2(rail_x, reader.center_y - 2.0f), color);
                }
            }
        }

    } // namespace

    bool supports_source_graph(const ObjectInfo& object) {
        return object.role == ObjectRole::Source || object.role == ObjectRole::CodeSource ||
               object.role == ObjectRole::DIPfileSource || object.role == ObjectRole::ProjectFile;
    }

    void draw_source_graph_view(ViewerModel& model, SourceNavigationState& navigation) {
        const auto* selected = model.object(model.selection());
        if (!selected || !supports_source_graph(*selected))
            return;
        const auto& relations = model.source_relations(*selected);
        ImGui::TextDisabled("Direct source reads; arrows point from a source to its reader.");
        ImGui::BeginChild("Sources graph canvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        const float rail_x = ImGui::GetCursorScreenPos().x + 13.0f;
        ImGui::Indent(39.0f);
        const float card_x = ImGui::GetCursorScreenPos().x;
        const float card_width = std::max(1.0f, ImGui::GetContentRegionAvail().x);
        std::optional<SourceTarget> clicked_target;
        std::string clicked_path;
        std::vector<Card> dependencies;
        std::vector<Card> readers;

        ImGui::SeparatorText("Read by this source");
        for (const auto& path : relations.depends_on) {
            const auto* object = model.object(path);
            if (!object)
                continue;
            const auto target = relations.dependency_targets.find(path);
            dependencies.push_back(draw_card(
                model, *object, target == relations.dependency_targets.end() ? nullptr : &target->second,
                false, card_width, clicked_target, clicked_path
            ));
        }
        if (dependencies.empty())
            ImGui::TextDisabled("No direct source reads recorded.");

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        const auto source_targets = model.source_targets(*selected);
        const auto center = draw_card(
            model, *selected, source_targets.empty() ? nullptr : &source_targets.front(),
            true, card_width, clicked_target, clicked_path
        );
        ImGui::Dummy(ImVec2(1.0f, 18.0f));

        ImGui::SeparatorText("Reads this source");
        for (const auto& path : relations.used_by) {
            const auto* object = model.object(path);
            if (!object)
                continue;
            const auto target = relations.reader_targets.find(path);
            readers.push_back(draw_card(
                model, *object, target == relations.reader_targets.end() ? nullptr : &target->second,
                false, card_width, clicked_target, clicked_path
            ));
        }
        if (readers.empty())
            ImGui::TextDisabled("No source reads this source directly.");

        draw_connections(
            dependencies, center, readers, rail_x, card_x,
            ImGui::GetColorU32(ImGuiCol_TextDisabled)
        );
        ImGui::Unindent(39.0f);
        ImGui::EndChild();
        if (clicked_target)
            navigation.navigate_to(model, *clicked_target);
        else if (!clicked_path.empty())
            model.select(clicked_path);
    }

} // namespace snt::view
