#include "application.h"
#include "source_view.h"
#include "viewer_model.h"

#include <snt/dip/dependency_graph.h>
#include <snt/core/datatypes.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_set>
#include <utility>
#include <vector>

namespace snt::view {
namespace {

constexpr char VIEWER_TITLE[] = "SNT3 Parameter Viewer v" CODE_VERSION;

void glfw_error(int code, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

bool matches(const ViewerModel& model, const ObjectInfo& object, const std::string& query) {
    if (query.empty() || object.path.find(query) != std::string::npos) return true;
    for (const auto& child : object.children)
        if (const auto* item = model.object(child); item && matches(model, *item, query)) return true;
    return false;
}

const char* object_kind_name(const ObjectInfo& object) {
    if (object.role == ObjectRole::Project) return "Evaluated project";
    if (object.role == ObjectRole::DIPfile) return "DIPfile manifest";
    if (object.role == ObjectRole::ManifestCategory) return "DIPfile category";
    if (object.role == ObjectRole::ManifestEntry) return "DIPfile entry";
    if (object.role == ObjectRole::Sources) return "Named sources";
    if (object.role == ObjectRole::Source) return "Named source";
    if (object.role == ObjectRole::Overrides) return "Overrides";
    if (object.role == ObjectRole::Override) return "Overridden value";
    if (object.role == ObjectRole::Schemas) return "Schemas";
    if (object.role == ObjectRole::Schema) return "Schema";
    if (object.role == ObjectRole::Units) return "Custom units";
    if (object.role == ObjectRole::Unit) return "Custom unit";
    if (object.has_value && !object.children.empty()) return "Value and children";
    if (object.has_value) return "Value";
    if (object.hierarchy_kind == dip::Path::Kind::Map) return "Map collection";
    if (object.hierarchy_kind == dip::Path::Kind::List) return "List collection";
    if (object.hierarchy_kind == dip::Path::Kind::Item) return "Collection item";
    return "Group";
}

void draw_kind_icon(const ObjectInfo& object, ImVec2 pos, float size) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
    const auto point = [&](float x, float y) { return ImVec2(pos.x + x * size, pos.y + y * size); };
    if (object.role == ObjectRole::Schemas || object.role == ObjectRole::Schema) {
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
    } else if (object.role == ObjectRole::Sources || object.role == ObjectRole::Source) {
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
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (item->children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (model.selection() == path) flags |= ImGuiTreeNodeFlags_Selected;
        if (!query.empty()) flags |= ImGuiTreeNodeFlags_DefaultOpen;
        if (item->role == ObjectRole::Project || item->role == ObjectRole::DIPfile ||
            item->role == ObjectRole::Sources ||
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

void labeled_text(const char* label, const std::string& value) {
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine(120.0f);
    ImGui::TextWrapped("%s", value.c_str());
}

struct InspectorCache {
    std::string path;
    std::size_t revision = 0;
    bool valid = false;
    std::vector<std::pair<const char*, std::string>> fields;
    std::vector<std::string> reads;
    std::vector<std::string> readers;
    std::vector<std::string> schema_values;
    std::vector<SourceTarget> source_targets;
    std::string error;
};

void refresh_inspector(ViewerModel& model, InspectorCache& cache) {
    cache = {};
    cache.path = model.selection();
    cache.revision = model.revision();
    cache.valid = true;
    const auto* object = model.object(model.selection());
    if (!object) return;
    cache.source_targets = model.source_targets(*object);
    cache.fields.emplace_back("Path", object->role == ObjectRole::Artifact ||
                                      object->role == ObjectRole::DIPfile ||
                                      object->role == ObjectRole::Project
                                          ? model.input_path()
                                          : object->role == ObjectRole::Override ||
                                            object->role == ObjectRole::Schema ||
                                            object->role == ObjectRole::Unit
                                              ? object->node_path : object->path);
    if (!object->source_name.empty()) {
        const auto& source = model.environment().sources.at(object->source_name);
        cache.fields.emplace_back("Source", source.path.empty() ? source.name
                                                                  : model.display_file_path(source.path));
    }
    if (object->role == ObjectRole::ManifestEntry && object->manifest_index) {
        const auto& entry = model.environment().project_entries().at(*object->manifest_index);
        cache.fields.emplace_back("Kind", "DIPfile registration");
        if (!entry.name.empty()) cache.fields.emplace_back("Name", entry.name);
        cache.fields.emplace_back("Declared", entry.resolved_path.empty()
            ? (entry.kind == dip::ProjectEntry::Kind::Unit ? entry.value : "Inline text")
            : entry.value);
        if (!entry.resolved_path.empty())
            cache.fields.emplace_back("Resolved file", model.display_file_path(entry.resolved_path));
        cache.fields.emplace_back("DIPfile line", std::to_string(entry.line));
        return;
    }
    if (object->role == ObjectRole::Schema) {
        cache.fields.emplace_back("Kind", object_kind_name(*object));
        const auto schemas = model.environment().get_schema_manifest();
        const auto info = std::find_if(schemas.begin(), schemas.end(),
            [&](const auto& schema) { return schema.name == object->node_path; });
        if (info != schemas.end()) {
            if (!info->metadata.description.empty())
                cache.fields.emplace_back("Description", info->metadata.description);
            if (!info->source_name.empty()) {
                const std::string location = info->source && !info->source->path.empty()
                    ? model.display_file_path(info->source->path) : info->source_name;
                cache.fields.emplace_back("Declared", location + ":" +
                                          std::to_string(info->source_line));
            }
            for (const auto& node : model.environment().nodes.get_nodes())
                if (node && node->value && node->schema_id == info->id)
                    cache.schema_values.push_back(node->path.name);
        }
        return;
    }
    if (object->role == ObjectRole::Unit) {
        cache.fields.emplace_back("Kind", object_kind_name(*object));
        cache.fields.emplace_back("Definition",
            model.environment().units.at(object->node_path).definition);
        return;
    }
    if (object->role == ObjectRole::Source) {
        const auto& named = model.environment().sources.at(object->source_name);
        cache.fields.emplace_back("Kind", named.raw_text ? "Raw source" : "Named DIPL source");
        if (named.raw_text)
            cache.fields.emplace_back("Size", std::to_string(named.code.size()) + " bytes");
        if (model.environment().dependency_graph().recorded)
            cache.readers = model.environment().dependency_graph().referenced_by(object->path);
        return;
    }
    if (!object->has_value) {
        cache.fields.emplace_back("Kind", object->path.empty() ? "Artifact" : object_kind_name(*object));
        return;
    }
    cache.fields.emplace_back("Kind", object_kind_name(*object));
    try {
        const auto node = model.value_node(*object);
        if (!node || !node->value) throw std::runtime_error("The selected value is unavailable");
        const auto shape = node->value->get_shape();
        const auto type = snt::core::DataTypeNames.find(node->value->get_dtype());
        if (type != snt::core::DataTypeNames.end()) cache.fields.emplace_back("Type", type->second);
        if (shape.empty() || node->value->get_size() == 1)
            cache.fields.emplace_back("Value", node->value->to_string());
        else
            cache.fields.emplace_back("Value", "Array (select a slice in a later numerical view)");
        if (!shape.empty()) {
            std::string dimensions;
            for (const auto dimension : shape) {
                if (!dimensions.empty()) dimensions += " × ";
                dimensions += std::to_string(dimension);
            }
            cache.fields.emplace_back("Shape", std::move(dimensions));
        }
        if (node->units) cache.fields.emplace_back("Units", node->units->to_string());
        if (!node->metadata.description.empty())
            cache.fields.emplace_back("Description", node->metadata.description);

        if (!object->source_name.empty()) {
            const auto& source = model.environment().sources.at(object->source_name);
            cache.fields.emplace_back("Declared", (source.path.empty() ? source.name
                                                                        : model.display_file_path(source.path)) + ":" +
                                       std::to_string(node->line.source.line_number));
        } else {
            const auto provenance = model.environment()[object->node_path].get_provenance();
            if (!provenance.source_name.empty()) {
                const std::string source = provenance.source && !provenance.source->path.empty()
                    ? model.display_file_path(provenance.source->path) : provenance.source_name;
                cache.fields.emplace_back("Declared", source + ":" + std::to_string(provenance.source_line));
            }
            if (provenance.override_line) {
                const std::string source = provenance.override_source && !provenance.override_source->path.empty()
                    ? model.display_file_path(provenance.override_source->path) : provenance.source_name;
                cache.fields.emplace_back("Override", source + ":" + std::to_string(provenance.override_line));
            }
            const auto schemas = model.environment().get_applied_schemas(object->node_path);
            if (!schemas.empty()) {
                std::string names;
                for (const auto& schema : schemas) {
                    if (!names.empty()) names += ", ";
                    names += schema.name;
                }
                cache.fields.emplace_back("Schemas", std::move(names));
            }
        }

        const auto& graph = model.environment().dependency_graph();
        if (graph.recorded) {
            const std::string graph_path = object->source_name.empty()
                ? "?" + object->node_path : object->path;
            for (const auto& read : graph.dependencies(graph_path)) cache.reads.push_back(read.target);
            cache.readers = graph.referenced_by(graph_path);
        }
    } catch (const std::exception& error) {
        cache.error = error.what();
    }
}

ImU32 syntax_color(SyntaxKind kind) {
    if (kind == SyntaxKind::Text) return ImGui::GetColorU32(ImGuiCol_Text);
    const auto rgb = dipl::highlight::color(kind);
    return IM_COL32(rgb.red, rgb.green, rgb.blue, 255);
}

void draw_source_view(const ViewerModel& model, const SourceView& source,
                      bool& scroll_to_target, const std::string& error) {
    ImGui::TextUnformatted(model.display_file_path(source.file().string()).c_str());
    ImGui::SameLine();
    ImGui::TextDisabled(":%zu  Read-only", source.target_line());
    if (ImGui::Button("Copy source")) ImGui::SetClipboardText(source.text().c_str());
    if (!error.empty()) ImGui::TextWrapped("%s", error.c_str());
    ImGui::Separator();
    ImGui::BeginChild("Source lines", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_HorizontalScrollbar);
    const float line_height = ImGui::GetTextLineHeightWithSpacing();
    const float glyph_height = ImGui::GetTextLineHeight();
    if (scroll_to_target && source.target_line() > 0) {
        const float target_y = static_cast<float>(source.target_line() - 1) * line_height;
        ImGui::SetScrollY(std::max(0.0f, target_y - ImGui::GetWindowHeight() * 0.45f));
        scroll_to_target = false;
    }
    const auto& lines = source.lines();
    const float number_width = ImGui::CalcTextSize(std::to_string(lines.size()).c_str()).x + 18.0f;
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(lines.size()), line_height);
    while (clipper.Step()) {
        for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index) {
            const auto& line = lines[static_cast<std::size_t>(index)];
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            const float width = std::max(ImGui::GetContentRegionAvail().x,
                number_width + ImGui::CalcTextSize(line.text.c_str(), nullptr, false).x + 20.0f);
            ImGui::Dummy(ImVec2(width, glyph_height));
            auto* draw = ImGui::GetWindowDrawList();
            if (static_cast<std::size_t>(index + 1) == source.target_line())
                draw->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + line_height),
                                    ImGui::GetColorU32(ImGuiCol_Header));
            const std::string number = std::to_string(index + 1);
            draw->AddText(ImVec2(pos.x + number_width - 12.0f - ImGui::CalcTextSize(number.c_str()).x,
                                 pos.y), ImGui::GetColorU32(ImGuiCol_TextDisabled), number.c_str());
            float x = pos.x + number_width;
            for (const auto& span : line.spans) {
                draw->AddText(ImVec2(x, pos.y), syntax_color(span.kind), span.text.c_str());
                x += ImGui::CalcTextSize(span.text.c_str(), nullptr, false).x;
            }
        }
    }
    ImGui::EndChild();
}

void draw_inspector(ViewerModel& model, InspectorCache& cache, SourceView& source,
                    bool& select_source_tab, bool& scroll_to_target, std::string& source_error) {
    if (!cache.valid || cache.path != model.selection() || cache.revision != model.revision())
        refresh_inspector(model, cache);
    if (const auto* object = model.object(model.selection())) {
        if (dip::detect_artifact(model.artifact()) == dip::ArtifactKind::DIPH5) {
            ImGui::BeginDisabled();
            ImGui::Button("Open source");
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Source text is not stored in .diph5 snapshots");
        } else {
            const auto& targets = cache.source_targets;
            for (std::size_t index = 0; index < targets.size(); ++index) {
                const auto& target = targets[index];
                if (index) ImGui::SameLine();
                std::error_code error;
                const bool available = std::filesystem::is_regular_file(target.file, error);
                ImGui::BeginDisabled(!available);
                const std::string label = targets.size() == 1 ? "Open source" : "Open " + target.label;
                const bool clicked = ImGui::Button(label.c_str());
                ImGui::EndDisabled();
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    const std::string location = model.display_file_path(target.file.string());
                    ImGui::SetTooltip("%s:%zu%s", location.c_str(), target.line,
                                      available ? "" : " (unavailable)");
                }
                if (clicked) {
                    if (source.open(target.file, target.line, source_error, target.plain_text)) {
                        const auto parsed = model.environment().sources.entries().find(target.source_name);
                        if (parsed != model.environment().sources.entries().end() &&
                            parsed->second.code != source.text())
                            source_error = "Source file changed since parsing; reload to update line locations.";
                        select_source_tab = true;
                        scroll_to_target = true;
                    }
                }
            }
        }
    }
    if (!source_error.empty() && !select_source_tab)
        ImGui::TextWrapped("%s", source_error.c_str());
    for (const auto& [label, value] : cache.fields) labeled_text(label, value);
    if (const auto* object = model.object(model.selection());
        object && object->role == ObjectRole::ManifestEntry && object->manifest_index) {
        const auto& entry = model.environment().project_entries().at(*object->manifest_index);
        std::string target;
        switch (entry.kind) {
        case dip::ProjectEntry::Kind::Unit: target = "@unit?" + entry.name; break;
        case dip::ProjectEntry::Kind::Schema: target = "@schema?" + entry.name; break;
        case dip::ProjectEntry::Kind::Source: target = entry.name + "?"; break;
        case dip::ProjectEntry::Kind::Code:
        case dip::ProjectEntry::Kind::Override: break;
        }
        if (!target.empty() && model.object(target)) {
            ImGui::Separator();
            if (ImGui::Selectable("Show registered object")) model.select(target);
        }
    }
    if (const auto* object = model.object(model.selection());
        object && object->role == ObjectRole::Override && model.object(object->node_path)) {
        ImGui::Separator();
        if (ImGui::Selectable("Show in evaluated project")) model.select(object->node_path);
    }
    if (!cache.schema_values.empty()) {
        ImGui::SeparatorText("Contributed values");
        for (const auto& path : cache.schema_values)
            if (ImGui::Selectable(path.c_str())) model.select(path);
    }
    if (!cache.reads.empty()) {
        ImGui::SeparatorText("Depends on");
        for (const auto& read : cache.reads) {
            const std::string target = read.size() > 1 && read.front() == '?' ? read.substr(1) : read;
            if (!target.empty() && model.object(target)) {
                if (ImGui::Selectable(read.c_str())) model.select(target);
            } else {
                ImGui::TextUnformatted(read.c_str());
            }
        }
    }
    if (!cache.readers.empty()) {
        ImGui::SeparatorText("Referenced by");
        for (const auto& reader : cache.readers) {
            const std::string target = reader.size() > 1 && reader.front() == '?'
                ? reader.substr(1) : reader;
            if (!target.empty() && model.object(target)) {
                if (ImGui::Selectable(reader.c_str())) model.select(target);
            } else {
                ImGui::TextUnformatted(reader.c_str());
            }
        }
    }
    if (!cache.error.empty()) ImGui::TextWrapped("Inspection failed: %s", cache.error.c_str());
}

void draw_view(ViewerModel& model, InspectorCache& inspector_cache, SourceView& source,
               bool& select_source_tab, bool& scroll_to_target, std::string& source_error) {
    static char search[256] = {};
    static TreeExpansionState expansion;
    const auto reload = [&] {
        if (model.reload()) {
            source = SourceView{};
            source_error.clear();
            select_source_tab = false;
            scroll_to_target = false;
        }
    };
    if (expansion.suppress_selection_open && model.selection() != expansion.selection_at_collapse)
        expansion.suppress_selection_open = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Reload", "Ctrl+R")) reload();
            if (ImGui::MenuItem("Close")) glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Navigate")) {
            if (ImGui::MenuItem("Back", nullptr, false, model.can_back())) model.back();
            if (ImGui::MenuItem("Forward", nullptr, false, model.can_forward())) model.forward();
            if (ImGui::MenuItem("Parent")) {
                const auto* current = model.object(model.selection());
                if (current) model.select(current->parent);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    const auto& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false) && !io.WantTextInput) reload();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("Viewer", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextUnformatted(model.input_path().c_str());
    if (model.stale()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.2f, 1.0f), "Last valid view — reload failed");
    }
    ImGui::Separator();
    if (ImGui::BeginTable("ViewerPanels", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV,
                          ImVec2(0.0f, -80.0f))) {
        ImGui::TableSetupColumn("Browser", ImGuiTableColumnFlags_WidthStretch, 0.34f);
        ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch, 0.66f);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::BeginChild("Browser", ImVec2(0.0f, 0.0f));
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
        ImGui::EndChild();
        ImGui::TableSetColumnIndex(1);
        ImGui::BeginChild("Inspector", ImVec2(0.0f, 0.0f));
        static std::string displayed_selection;
        const bool selection_changed = displayed_selection != model.selection();
        displayed_selection = model.selection();
        if (ImGui::BeginTabBar("Inspector tabs")) {
            if (ImGui::BeginTabItem("Inspector", nullptr,
                                    selection_changed ? ImGuiTabItemFlags_SetSelected : 0)) {
                draw_inspector(model, inspector_cache, source, select_source_tab,
                               scroll_to_target, source_error);
                ImGui::EndTabItem();
            }
            if (!source.file().empty() &&
                ImGui::BeginTabItem("Source", nullptr,
                    select_source_tab ? ImGuiTabItemFlags_SetSelected : 0)) {
                draw_source_view(model, source, scroll_to_target, source_error);
                ImGui::EndTabItem();
            }
            select_source_tab = false;
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        ImGui::EndTable();
    }
    ImGui::SeparatorText("Diagnostics");
    if (model.error().empty()) ImGui::TextDisabled("No reload error.");
    else ImGui::TextWrapped("%s", model.error().c_str());
    ImGui::End();
}

} // namespace

int run_application(ViewerModel& model) {
    glfwSetErrorCallback(glfw_error);
    if (!glfwInit()) return 1;
#ifdef __APPLE__
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1200, 760, VIEWER_TITLE, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    const float ui_scale = std::max(1.0f, ImGui_ImplGlfw_GetContentScaleForWindow(window));
    style.ScaleAllSizes(ui_scale);
    style.FontScaleDpi = ui_scale;
    style.FontSizeBase = 16.0f;
    io.Fonts->AddFontDefaultVector();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    InspectorCache inspector_cache;
    SourceView source;
    bool select_source_tab = false;
    bool scroll_to_target = false;
    std::string source_error;
    while (!glfwWindowShouldClose(window)) {
        // Input wakes the frame immediately; the timeout keeps idle UI updates active.
        glfwWaitEventsTimeout(1.0 / 60.0);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        draw_view(model, inspector_cache, source, select_source_tab, scroll_to_target, source_error);
        ImGui::Render();
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.94f, 0.95f, 0.96f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

} // namespace snt::view
