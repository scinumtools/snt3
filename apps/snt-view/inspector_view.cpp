#include "inspector_view.h"
#include "browser_view.h"
#include "source_view.h"

#include <snt/core/datatypes.h>
#include <snt/dip/inspect/dependency_graph.h>

#include <imgui.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace snt::view {
namespace {

void labeled_text(const char* label, const std::string& value) {
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine(120.0f);
    ImGui::TextWrapped("%s", value.c_str());
}

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
                                          : object->role == ObjectRole::CodeSource
                                              ? object->label
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
        cache.fields.emplace_back("Kind", named.table_text ? "Raw table source" :
            named.raw_text ? "Raw source" : "Named DIPL source");
        if (named.raw_text)
            cache.fields.emplace_back("Size", std::to_string(named.code.size()) + " bytes");
        if (model.environment().dependency_graph().recorded)
            cache.readers = model.environment().dependency_graph().referenced_by(object->path);
        return;
    }
    if (object->role == ObjectRole::CodeSource && object->manifest_index) {
        const auto& entry = model.environment().project_entries().at(*object->manifest_index);
        cache.fields.emplace_back("Kind", entry.resolved_path.empty()
            ? "Inline local source" : "Local source file");
        cache.fields.emplace_back("Source", entry.resolved_path.empty()
            ? model.display_file_path(model.artifact().string()) + ":" + std::to_string(entry.line)
            : model.display_file_path(entry.resolved_path));
        return;
    }
    if (object->role == ObjectRole::BlockSource) {
        const auto& input = dip::inspect_block_inputs(model.environment()).at(object->node_path);
        cache.fields.emplace_back("Kind", input.kind == dip::BlockInput::Kind::Table
            ? "Table block value" : "Array block value");
        cache.fields.emplace_back("Data", "Open the Data tab to browse evaluated values");
        if (!cache.source_targets.empty()) {
            const auto& target = cache.source_targets.front();
            cache.fields.emplace_back("Declared", model.display_file_path(target.file.string()) + ":" +
                std::to_string(target.line));
        }
        return;
    }
    if (!object->has_value) {
        cache.fields.emplace_back("Kind", object->path.empty() ? "Artifact" : object_kind_name(*object));
        if (object->has_table) {
            const auto table = dip::inspect_table(model.environment(), object->node_path);
            cache.fields.emplace_back("Rows", std::to_string(table.rows));
            cache.fields.emplace_back("Columns", std::to_string(table.columns.size()));
            cache.fields.emplace_back("Data", "Open the Data tab to browse table values");
        }
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
        else {
            cache.fields.emplace_back("Value", "Array (open Data to inspect bounded slices)");
            cache.fields.emplace_back("Elements", std::to_string(node->value->get_size()));
        }
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

} // namespace

void draw_source_view(const ViewerModel& model, const SourceView& source,
                      bool& scroll_to_target, const std::string& error) {
    ImGui::TextUnformatted(model.display_file_path(source.file().string()).c_str());
    ImGui::SameLine();
    ImGui::TextDisabled(":%zu  Read-only", source.source_line());
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
                const bool available = target.block_path.has_value() ||
                    std::filesystem::is_regular_file(target.file, error);
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
                    const bool opened = target.block_path
                        ? source.open_text(target.file, target.line,
                                           dip::inspect_block_inputs(model.environment()).at(*target.block_path).code,
                                           target.format, source_error)
                        : source.open(target.file, target.line, source_error, target.format);
                    if (opened) {
                        const auto parsed = model.environment().sources.entries().find(target.source_name);
                        if (!target.block_path && parsed != model.environment().sources.entries().end() &&
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
            target = "@code?" + std::to_string(*object->manifest_index);
            break;
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
        if (ImGui::Selectable("Show in resolved nodes")) model.select(object->node_path);
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

} // namespace snt::view
