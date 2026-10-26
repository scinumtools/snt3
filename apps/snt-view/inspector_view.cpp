#include "inspector_view.h"

#include "browser_view.h"
#include "notifications.h"
#include "source_view.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <snt/core/datatypes.h>
#include <snt/dip/artifact.h>
#include <snt/dip/inspect/dependency_graph.h>
#include <snt/dip/parsers.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_set>
#include <utility>

namespace snt::view {
    namespace {

        int resize_string_callback(ImGuiInputTextCallbackData* data) {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                auto* text = static_cast<std::string*>(data->UserData);
                text->resize(static_cast<std::size_t>(data->BufTextLen));
                data->Buf = text->data();
            }
            return 0;
        }

        std::string preview_report(const dip::PreviewResult& result) {
            std::string report;
            if (!result.candidate_valid) {
                report = "Candidate has errors.";
                for (const auto& diagnostic : result.candidate_diagnostics) {
                    report += "\n";
                    if (diagnostic.location) {
                        if (diagnostic.location->line)
                            report += "Line " + std::to_string(diagnostic.location->line);
                        else if (!diagnostic.location->source.empty())
                            report += diagnostic.location->source;
                        if (diagnostic.location->column)
                            report += ":" + std::to_string(*diagnostic.location->column);
                        if (diagnostic.location->line || !diagnostic.location->source.empty())
                            report += ": ";
                    }
                    report += diagnostic.code + ": " + diagnostic.message;
                    if (!diagnostic.details.empty())
                        report += "\n" + diagnostic.details;
                    if (!diagnostic.suggestion.empty())
                        report += "\nSuggestion: " + diagnostic.suggestion;
                }
                return report;
            }
            report = "Candidate parses and evaluates successfully.";
            report += "\n" + dip::render_comparison(result.comparison);
            return report;
        }

        std::string block_value_report(const dip::BlockInput& input, const std::string& text) {
            try {
                if (input.kind == dip::BlockInput::Kind::Array) {
                    std::string normalized = text;
                    normalized.erase(std::remove(normalized.begin(), normalized.end(), '\n'), normalized.end());
                    const auto first = std::find_if_not(normalized.begin(), normalized.end(), [](unsigned char ch) {
                        return std::isspace(ch);
                    });
                    const auto last = std::find_if_not(normalized.rbegin(), normalized.rend(), [](unsigned char ch) {
                        return std::isspace(ch);
                    }).base();
                    normalized = first < last ? std::string(first, last) : std::string{};
                    val::Array::StringType values;
                    val::Array::ShapeType shape;
                    const std::string parsed = dip::parse_array(normalized, values, shape);
                    if (std::any_of(parsed.size() < normalized.size() ? normalized.begin() + parsed.size()
                                                                      : normalized.end(),
                                    normalized.end(), [](unsigned char ch) { return !std::isspace(ch); }))
                        return "Unexpected text after the closing array bracket.";
                    return "Array block value syntax is valid.";
                }

                std::queue<dip::Line> lines;
                lines = dip::parse_lines(lines, text, input.source_name);
                bool has_separator = false;
                auto remaining = lines;
                while (!remaining.empty()) {
                    std::string line = remaining.front().code;
                    remaining.pop();
                    const auto first = line.find_first_not_of(" \t\r\n");
                    if (first == std::string::npos)
                        line.clear();
                    else
                        line = line.substr(first, line.find_last_not_of(" \t\r\n") - first + 1);
                    if (line == "---") {
                        has_separator = true;
                        break;
                    }
                }
                if (!has_separator)
                    return "Table block value is missing the `---` header separator.";

                dip::ValueMetadata metadata;
                const auto columns = dip::parse_table_nodes(lines, metadata);
                if (columns.empty())
                    return "Table block value must define at least one column.";
                return "Table block value syntax is valid.";
            } catch (const std::exception& exception) {
                return exception.what();
            }
        }

        void labeled_text(const char* label, const std::string& value) {
            ImGui::TextDisabled("%s", label);
            ImGui::SameLine(120.0f);
            ImGui::TextWrapped("%s", value.c_str());
        }

        bool same_file(const std::string& lhs, const std::string& rhs) {
            const auto normalized = [](const std::string& value) {
                std::error_code error;
                const auto canonical = std::filesystem::weakly_canonical(value, error);
                if (!error)
                    return canonical;
                error.clear();
                const auto absolute = std::filesystem::absolute(value, error);
                return error ? std::filesystem::path(value).lexically_normal() : absolute.lexically_normal();
            };
            return normalized(lhs) == normalized(rhs);
        }

        void refresh_inspector(ViewerModel& model, InspectorCache& cache) {
            cache = {};
            cache.path = model.selection();
            cache.revision = model.revision();
            cache.valid = true;
            const auto* object = model.object(model.selection());
            if (!object)
                return;
            const dip::Inspector view{model.environment()};
            cache.source_targets = model.source_targets(*object);
            cache.fields.emplace_back(
                "Path",
                object->role == ObjectRole::Artifact || object->role == ObjectRole::DIPfile ||
                        object->role == ObjectRole::Project
                    ? model.input_path()
                : object->role == ObjectRole::CodeSource ? object->label
                : object->role == ObjectRole::Override || object->role == ObjectRole::Schema ||
                        object->role == ObjectRole::Unit
                    ? object->node_path
                    : object->path
            );
            if (!object->source_name.empty()) {
                const auto& source = model.environment().sources.at(object->source_name);
                cache.fields.emplace_back(
                    "Source", source.path.empty() ? source.name : model.display_file_path(source.path)
                );
            }
            if (object->role == ObjectRole::ManifestEntry && object->manifest_index) {
                const auto& entry = model.environment().project_entries().at(*object->manifest_index);
                cache.fields.emplace_back("Kind", "DIPfile registration");
                if (!entry.name.empty())
                    cache.fields.emplace_back("Name", entry.name);
                cache.fields.emplace_back(
                    "Declared",
                    entry.resolved_path.empty()
                        ? (entry.kind == dip::ProjectEntry::Kind::Unit ? entry.value : "Inline text")
                        : entry.value
                );
                if (!entry.resolved_path.empty())
                    cache.fields.emplace_back("Resolved file", model.display_file_path(entry.resolved_path));
                cache.fields.emplace_back("DIPfile line", std::to_string(entry.line));
                return;
            }
            if (object->role == ObjectRole::Schema) {
                cache.fields.emplace_back("Kind", object_kind_name(*object));
                const auto schemas = model.environment().get_schema_manifest();
                const auto info = std::find_if(schemas.begin(), schemas.end(), [&](const auto& schema) {
                    return schema.name == object->node_path;
                });
                if (info != schemas.end()) {
                    if (!info->metadata.description.empty())
                        cache.fields.emplace_back("Description", info->metadata.description);
                    if (!info->source_name.empty()) {
                        const std::string location = info->source && !info->source->path.empty()
                                                         ? model.display_file_path(info->source->path)
                                                         : info->source_name;
                        cache.fields.emplace_back("Declared", location + ":" + std::to_string(info->source_line));
                    }
                    for (const auto& node : model.environment().nodes.get_nodes())
                        if (node && node->value && node->schema_id == info->id)
                            cache.schema_values.push_back(node->path.name);
                }
                return;
            }
            if (object->role == ObjectRole::Unit) {
                cache.fields.emplace_back("Kind", object_kind_name(*object));
                cache.fields.emplace_back("Definition", model.environment().units.at(object->node_path).definition);
                return;
            }
            if (object->role == ObjectRole::Source) {
                const auto& named = model.environment().sources.at(object->source_name);
                cache.fields.emplace_back(
                    "Kind",
                    named.table_text ? "Raw table source"
                    : named.raw_text ? "Raw source"
                                     : "Named DIPL source"
                );
                if (named.raw_text)
                    cache.fields.emplace_back("Size", std::to_string(named.code.size()) + " bytes");
                if (view.graph().recorded)
                    cache.readers = view.graph().referenced_by(object->path);
                for (const auto& node : named.nodes.get_nodes())
                    if (node && !node->path.name.empty())
                        cache.source_nodes.emplace_back(
                            node->path.name, object->source_name + "?" + node->path.name
                        );
                if (named.table_text && !named.table_path.empty()) {
                    const std::string prefix = named.table_path + ".";
                    for (const auto& node : model.environment().nodes.get_nodes()) {
                        if (!node || node->table_path != named.table_path ||
                            node->path.name.compare(0, prefix.size(), prefix) != 0)
                            continue;
                        cache.source_nodes.emplace_back(node->path.name, node->path.name);
                    }
                }
                return;
            }
            if (object->role == ObjectRole::CodeSource && object->manifest_index) {
                const auto& entry = model.environment().project_entries().at(*object->manifest_index);
                cache.fields.emplace_back(
                    "Kind",
                    entry.kind == dip::ProjectEntry::Kind::Override ? "Override source file"
                    : entry.resolved_path.empty() ? "Inline local source"
                                                  : "Local source file"
                );
                cache.fields.emplace_back(
                    "Source",
                    entry.resolved_path.empty()
                        ? model.display_file_path(model.artifact().string()) + ":" + std::to_string(entry.line)
                        : model.display_file_path(entry.resolved_path)
                );
                if (!entry.resolved_path.empty()) {
                    const auto& sources = model.environment().sources.entries();
                    const auto source_matches_file = [&](const std::string& source_name) {
                        const auto source = sources.find(source_name);
                        return source != sources.end() && !source->second.path.empty() &&
                               same_file(source->second.path, entry.resolved_path);
                    };
                    std::unordered_set<std::string> listed_nodes;
                    for (const auto& node : model.environment().nodes.get_nodes()) {
                        if (!node || node->path.name.empty())
                            continue;
                        bool belongs_to_file = source_matches_file(node->line.source.name) ||
                                                source_matches_file(node->override_line.source.name);
                        if (!belongs_to_file)
                            belongs_to_file = std::any_of(
                                node->modification_lines.begin(),
                                node->modification_lines.end(),
                                [&](const auto& modification) {
                                    return source_matches_file(modification.source.name);
                                }
                            );
                        if (belongs_to_file && listed_nodes.insert(node->path.name).second)
                            cache.source_nodes.emplace_back(node->path.name, node->path.name);
                    }
                }
                return;
            }
            if (object->role == ObjectRole::DIPfileSource) {
                cache.fields.emplace_back("Kind", "DIPfile manifest source");
                cache.fields.emplace_back("Source", model.display_file_path(model.artifact().string()));
                return;
            }
            if (object->role == ObjectRole::BlockSource) {
                const auto& input = view.block_inputs().at(object->node_path);
                cache.fields.emplace_back(
                    "Kind", input.kind == dip::BlockInput::Kind::Table ? "Table block value" : "Array block value"
                );
                cache.fields.emplace_back("Data", "Open the Data tab to browse evaluated values");
                if (!cache.source_targets.empty()) {
                    const auto& target = cache.source_targets.front();
                    cache.fields.emplace_back(
                        "Declared", model.display_file_path(target.file.string()) + ":" + std::to_string(target.line)
                    );
                }
                const std::string prefix = object->node_path + ".";
                for (const auto& node : model.environment().nodes.get_nodes()) {
                    if (!node)
                        continue;
                    if (input.kind == dip::BlockInput::Kind::Table) {
                        if (node->table_path == object->node_path &&
                            node->path.name.compare(0, prefix.size(), prefix) == 0)
                            cache.source_nodes.emplace_back(node->path.name, node->path.name);
                    } else if (node->path.name == object->node_path) {
                        cache.source_nodes.emplace_back(node->path.name, node->path.name);
                    }
                }
                return;
            }
            if (!object->has_value) {
                cache.fields.emplace_back("Kind", object->path.empty() ? "Artifact" : object_kind_name(*object));
                if (object->has_table) {
                    const auto table = view.table(object->node_path);
                    cache.fields.emplace_back("Rows", std::to_string(table.rows));
                    cache.fields.emplace_back("Columns", std::to_string(table.columns.size()));
                    cache.fields.emplace_back("Data", "Open the Data tab to browse table values");
                }
                return;
            }
            cache.fields.emplace_back("Kind", object_kind_name(*object));
            try {
                const auto node = model.value_node(*object);
                if (!node || !node->value)
                    throw std::runtime_error("The selected value is unavailable");
                const auto shape = node->value->get_shape();
                const auto type = snt::core::DataTypeNames.find(node->value->get_dtype());
                if (type != snt::core::DataTypeNames.end())
                    cache.fields.emplace_back("Type", type->second);
                if (shape.empty() || node->value->get_size() == 1)
                    cache.fields.emplace_back("Value", node->value->to_string());
                else {
                    cache.fields.emplace_back("Value", "Array (open Data to inspect bounded slices)");
                    cache.fields.emplace_back("Elements", std::to_string(node->value->get_size()));
                }
                if (!shape.empty()) {
                    std::string dimensions;
                    for (const auto dimension : shape) {
                        if (!dimensions.empty())
                            dimensions += " × ";
                        dimensions += std::to_string(dimension);
                    }
                    cache.fields.emplace_back("Shape", std::move(dimensions));
                }
                if (node->units)
                    cache.fields.emplace_back("Units", node->units->to_string());
                if (!node->metadata.description.empty())
                    cache.fields.emplace_back("Description", node->metadata.description);

                if (!object->source_name.empty()) {
                    const auto& source = model.environment().sources.at(object->source_name);
                    cache.fields.emplace_back(
                        "Declared",
                        (source.path.empty() ? source.name : model.display_file_path(source.path)) + ":" +
                            std::to_string(node->line.source.line_number)
                    );
                } else {
                    const auto provenance = model.environment()[object->node_path].get_provenance();
                    if (!provenance.source_name.empty()) {
                        const std::string source = provenance.source && !provenance.source->path.empty()
                                                       ? model.display_file_path(provenance.source->path)
                                                       : provenance.source_name;
                        cache.fields.emplace_back("Declared", source + ":" + std::to_string(provenance.source_line));
                    }
                    if (provenance.override_line) {
                        const std::string source =
                            provenance.override_source && !provenance.override_source->path.empty()
                                ? model.display_file_path(provenance.override_source->path)
                                : provenance.source_name;
                        cache.fields.emplace_back("Override", source + ":" + std::to_string(provenance.override_line));
                    }
                    const auto schemas = model.environment().get_applied_schemas(object->node_path);
                    if (!schemas.empty()) {
                        std::string names;
                        for (const auto& schema : schemas) {
                            if (!names.empty())
                                names += ", ";
                            names += schema.name;
                        }
                        cache.fields.emplace_back("Schemas", std::move(names));
                    }
                }

                const auto& graph = view.graph();
                if (graph.recorded) {
                    const std::string graph_path = object->source_name.empty() ? "?" + object->node_path : object->path;
                    for (const auto& read : graph.dependencies(graph_path))
                        cache.reads.push_back(read.target);
                    cache.readers = graph.referenced_by(graph_path);
                }
            } catch (const std::exception& error) {
                cache.error = error.what();
            }
        }

        ImU32 syntax_color(SyntaxKind kind) {
            if (kind == SyntaxKind::Text)
                return ImGui::GetColorU32(ImGuiCol_Text);
            const auto rgb = dipl::highlight::color(kind);
            return IM_COL32(rgb.red, rgb.green, rgb.blue, 255);
        }

    } // namespace

    void draw_source_view(
        ViewerModel& model,
        SourceView& source,
        bool& scroll_to_target,
        std::string& error,
        std::string& edit_report,
        bool& request_save_as,
        NotificationCenter& notifications
    ) {
        const auto* selected = model.object(model.selection());
        const bool block_value = selected && selected->role == ObjectRole::BlockSource;
        ImGui::TextUnformatted(model.display_file_path(source.file().string()).c_str());
        ImGui::SameLine();
        const auto edit_path = model.editable_document_path(source.file());
        ImGui::TextDisabled(":%zu  %s", source.source_line(), edit_path ? "Project source" : "Read-only");
        if (source.dirty()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.72f, 0.25f, 1.0f), "Modified");
        }
        if (ImGui::Button("Copy source")) {
            ImGui::SetClipboardText(source.text().c_str());
            notifications.notify("Source copied to clipboard.");
        }
        if (block_value && !source.editing()) {
            ImGui::SameLine();
            if (ImGui::Button("Check")) {
                const auto& input = dip::Inspector{model.environment()}.block_inputs().at(selected->node_path);
                edit_report = block_value_report(input, source.text());
                const bool valid = edit_report.find("syntax is valid.") != std::string::npos;
                notifications.notify(edit_report, valid ? NoticeLevel::Info : NoticeLevel::Error);
            }
        }
        if (edit_path) {
            ImGui::SameLine();
            if (!source.editing()) {
                if (ImGui::Button("Edit")) {
                    source.set_editing(true);
                    edit_report.clear();
                }
            } else {
                if (ImGui::Button("Check")) {
                    try {
                        if (block_value) {
                            const auto& input = dip::Inspector{model.environment()}.block_inputs().at(selected->node_path);
                            edit_report = block_value_report(input, source.text());
                        } else {
                            edit_report = preview_report(model.preview_document(*edit_path, source.text()));
                        }
                    } catch (const std::exception& exception) {
                        edit_report = exception.what();
                    }
                    const auto check_level = edit_report.find("syntax is valid.") != std::string::npos ||
                                                     edit_report.find("Candidate parses and evaluates successfully.") == 0
                                                 ? NoticeLevel::Info
                                                 : NoticeLevel::Error;
                    notifications.notify(edit_report, check_level);
                }
                ImGui::SameLine();
                ImGui::BeginDisabled(!source.dirty());
                const bool save = ImGui::Button("Save");
                ImGui::EndDisabled();
                if (save) {
                    edit_report.clear();
                    std::string document = source.text();
                    try {
                        if (block_value)
                            document = model.document_text_with_block(selected->node_path, source.text());
                    } catch (const std::exception& exception) {
                        edit_report = exception.what();
                    }
                    if (edit_report.empty() && model.save_document(*edit_path, document)) {
                        source.mark_saved();
                        edit_report = model.error().empty()
                                          ? "Saved and reloaded."
                                          : "Saved; reload could not evaluate the new text: " + model.error();
                    } else if (edit_report.empty())
                        edit_report = model.error();
                }
                ImGui::SameLine();
                ImGui::BeginDisabled(!model.can_save_as() || block_value);
                if (ImGui::Button("Save As..."))
                    request_save_as = true;
                ImGui::EndDisabled();
                ImGui::SameLine();
                if (ImGui::Button("Discard")) {
                    source.set_text(block_value
                                        ? dip::Inspector{model.environment()}.block_inputs().at(selected->node_path).code
                                        : model.document_text(*edit_path));
                    source.mark_saved();
                    source.set_editing(false);
                    edit_report.clear();
                    error.clear();
                }
            }
        }
        notifications.observe("source-view-error", error, NoticeLevel::Warning);
        const auto starts_with = [&](const char* prefix) {
            return edit_report.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
        };
        const auto report_level = starts_with("Saved and reloaded.") ||
                                          starts_with("Candidate parses and evaluates successfully")
                                      ? NoticeLevel::Info
                                      : starts_with("Saved; reload could not evaluate")
                                            ? NoticeLevel::Warning
                                            : NoticeLevel::Error;
        if (!block_value)
            notifications.observe("edit-report", edit_report, report_level);
        ImGui::Separator();
        if (edit_path && source.editing()) {
            std::string buffer = source.text();
            buffer.reserve(std::max<std::size_t>(buffer.capacity(), 4096));
            const ImVec2 editor_size = ImGui::GetContentRegionAvail();
            const auto& lines = source.lines();
            // InputTextMultiline lays out each hard line at the font size and
            // does not add ImGui's normal item spacing between lines. Match
            // that exact row pitch in the gutter as well.
            const float line_height = ImGui::GetFontSize();
            const float gutter_width =
                ImGui::CalcTextSize(std::to_string(std::max<std::size_t>(1, lines.size())).c_str()).x + 14.0f;
            const ImGuiID editor_id = ImGui::GetID("##SourceEditor");
            char editor_window_name[512];
            std::snprintf(
                editor_window_name,
                sizeof(editor_window_name),
                "%s/##SourceEditor_%08X",
                ImGui::GetCurrentWindow()->Name,
                editor_id
            );
            const ImGuiWindow* editor_window = ImGui::FindWindowByName(editor_window_name);
            ImGui::BeginChild(
                "##SourceLineNumbers",
                ImVec2(gutter_width, editor_size.y),
                ImGuiChildFlags_Borders,
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs
            );
            if (editor_window)
                ImGui::SetScrollY(editor_window->Scroll.y);
            ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
            const ImVec2 item_spacing = ImGui::GetStyle().ItemSpacing;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(item_spacing.x, 0.0f));
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(lines.size()), line_height);
            while (clipper.Step()) {
                for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index) {
                    const auto number = std::to_string(index + 1);
                    const ImVec2 position = ImGui::GetCursorScreenPos();
                    const float number_width = ImGui::CalcTextSize(number.c_str()).x;
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(position.x + gutter_width - number_width - 8.0f, position.y),
                        ImGui::GetColorU32(ImGuiCol_TextDisabled),
                        number.c_str()
                    );
                    ImGui::Dummy(ImVec2(gutter_width - 4.0f, line_height));
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndChild();
            ImGui::SameLine(0.0f, 0.0f);
            const bool changed = ImGui::InputTextMultiline(
                    "##SourceEditor",
                    buffer.data(),
                    buffer.capacity() + 1,
                    ImVec2(editor_size.x - gutter_width, editor_size.y),
                    ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AllowTabInput,
                    resize_string_callback,
                    &buffer
                );
            buffer.resize(std::strlen(buffer.c_str()));
            if (changed)
                source.set_text(std::move(buffer));
            return;
        }
        ImGui::BeginChild(
            "Source lines", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar
        );
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
                const float width = std::max(
                    ImGui::GetContentRegionAvail().x,
                    number_width + ImGui::CalcTextSize(line.text.c_str(), nullptr, false).x + 20.0f
                );
                ImGui::Dummy(ImVec2(width, glyph_height));
                auto* draw = ImGui::GetWindowDrawList();
                if (static_cast<std::size_t>(index + 1) == source.target_line())
                    draw->AddRectFilled(
                        pos, ImVec2(pos.x + width, pos.y + line_height), ImGui::GetColorU32(ImGuiCol_Header)
                    );
                const std::string number = std::to_string(index + 1);
                draw->AddText(
                    ImVec2(pos.x + number_width - 12.0f - ImGui::CalcTextSize(number.c_str()).x, pos.y),
                    ImGui::GetColorU32(ImGuiCol_TextDisabled),
                    number.c_str()
                );
                float x = pos.x + number_width;
                for (const auto& span : line.spans) {
                    draw->AddText(ImVec2(x, pos.y), syntax_color(span.kind), span.text.c_str());
                    x += ImGui::CalcTextSize(span.text.c_str(), nullptr, false).x;
                }
            }
        }
        ImGui::EndChild();
    }

    void draw_inspector(
        ViewerModel& model,
        InspectorCache& cache,
        SourceView& source,
        bool& select_source_tab,
        bool& scroll_to_target,
        std::string& source_error,
        NotificationCenter& notifications
    ) {
        const dip::Inspector view{model.environment()};
        if (!cache.valid || cache.path != model.selection() || cache.revision != model.revision())
            refresh_inspector(model, cache);
        if (const auto* object = model.object(model.selection())) {
            const bool selected_source = object->role == ObjectRole::Source || object->role == ObjectRole::CodeSource ||
                                         object->role == ObjectRole::BlockSource ||
                                         object->role == ObjectRole::DIPfileSource;
            if (!selected_source && dip::detect_artifact(model.artifact()) == dip::ArtifactKind::DIPH5) {
                ImGui::BeginDisabled();
                ImGui::Button("Open source");
                ImGui::EndDisabled();
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Source text is not stored in .diph5 snapshots");
            } else if (!selected_source) {
                const auto& targets = cache.source_targets;
                for (std::size_t index = 0; index < targets.size(); ++index) {
                    const auto& target = targets[index];
                    if (index)
                        ImGui::SameLine();
                    std::error_code error;
                    const bool available =
                        target.block_path.has_value() || std::filesystem::is_regular_file(target.file, error);
                    ImGui::BeginDisabled(!available);
                    const std::string label = targets.size() == 1 ? "Open source" : "Open " + target.label;
                    const bool clicked = ImGui::Button(label.c_str());
                    ImGui::EndDisabled();
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        const std::string location = model.display_file_path(target.file.string());
                        ImGui::SetTooltip("%s:%zu%s", location.c_str(), target.line, available ? "" : " (unavailable)");
                    }
                    if (clicked) {
                        const std::string source_path = model.source_object_path(target);
                        if (source_path.empty()) {
                            source_error = "The source is not listed in the Sources section.";
                            continue;
                        }
                        if (source.dirty()) {
                            if (source.file() != target.file) {
                                source_error = "Save or discard the current document before opening another source.";
                                continue;
                            }
                        }
                        if (model.select(source_path)) {
                            source_error.clear();
                            select_source_tab = true;
                            scroll_to_target = true;
                        }
                    }
                }
            }
        }
        notifications.observe("source-navigation-error", source_error, NoticeLevel::Warning);
        for (const auto& [label, value] : cache.fields)
            labeled_text(label, value);
        if (!cache.source_nodes.empty()) {
            ImGui::SeparatorText("Nodes");
            for (const auto& [label, target] : cache.source_nodes)
                if (ImGui::Selectable(label.c_str()))
                    model.select(target);
        }
        if (const auto* object = model.object(model.selection());
            object && object->role == ObjectRole::ManifestEntry && object->manifest_index) {
            const auto& entry = model.environment().project_entries().at(*object->manifest_index);
            std::string target;
            switch (entry.kind) {
            case dip::ProjectEntry::Kind::Unit:
                target = "@unit?" + entry.name;
                break;
            case dip::ProjectEntry::Kind::Schema:
                target = "@schema?" + entry.name;
                break;
            case dip::ProjectEntry::Kind::Source:
                target = entry.name + "?";
                break;
            case dip::ProjectEntry::Kind::Code:
                target = "@code?" + std::to_string(*object->manifest_index);
                break;
            case dip::ProjectEntry::Kind::Override:
                break;
            }
            if (!target.empty() && model.object(target)) {
                ImGui::Separator();
                if (ImGui::Selectable("Show registered object"))
                    model.select(target);
            }
        }
        if (const auto* object = model.object(model.selection());
            object && object->role == ObjectRole::Override && model.object(object->node_path)) {
            ImGui::Separator();
            if (ImGui::Selectable("Show in resolved nodes"))
                model.select(object->node_path);
        }
        if (!cache.schema_values.empty()) {
            ImGui::SeparatorText("Contributed values");
            for (const auto& path : cache.schema_values)
                if (ImGui::Selectable(path.c_str()))
                    model.select(path);
        }
        if (!cache.reads.empty()) {
            ImGui::SeparatorText("Depends on");
            for (const auto& read : cache.reads) {
                const std::string target = read.size() > 1 && read.front() == '?' ? read.substr(1) : read;
                if (!target.empty() && model.object(target)) {
                    if (ImGui::Selectable(read.c_str()))
                        model.select(target);
                } else {
                    ImGui::TextUnformatted(read.c_str());
                }
            }
        }
        if (!cache.readers.empty()) {
            ImGui::SeparatorText("Referenced by");
            for (const auto& reader : cache.readers) {
                const std::string target = reader.size() > 1 && reader.front() == '?' ? reader.substr(1) : reader;
                if (!target.empty() && model.object(target)) {
                    if (ImGui::Selectable(reader.c_str()))
                        model.select(target);
                } else {
                    ImGui::TextUnformatted(reader.c_str());
                }
            }
        }
        notifications.observe("inspector-error", cache.error, NoticeLevel::Error);
    }

    bool open_selected_source(ViewerModel& model, SourceView& source, std::string& error) {
        const auto* selected = model.object(model.selection());
        if (!selected || (selected->role != ObjectRole::Source && selected->role != ObjectRole::CodeSource &&
                          selected->role != ObjectRole::BlockSource && selected->role != ObjectRole::DIPfileSource))
            return false;
        const auto targets = model.source_targets(*selected);
        if (targets.empty()) {
            error = "No source text is available for this entry.";
            return false;
        }
        const auto& target = targets.front();
        if (source.dirty()) {
            if (source.file() == target.file)
                return true;
            error = "Save or discard the current document before opening another source.";
            return false;
        }
        bool opened = false;
        if (target.block_path) {
            const dip::Inspector view{model.environment()};
            opened = source.open_text(
                target.file,
                target.line,
                view.block_inputs().at(*target.block_path).code,
                target.format,
                error
            );
        } else if (const auto edit_path = model.editable_document_path(target.file)) {
            opened = source.open_document(
                target.file, target.line, model.document_text(*edit_path), target.format, error
            );
        } else {
            opened = source.open(target.file, target.line, error, target.format);
        }
        if (!opened)
            return false;
        const auto parsed = model.environment().sources.entries().find(target.source_name);
        if (!target.block_path && !model.editable_document_path(target.file) &&
            parsed != model.environment().sources.entries().end() && parsed->second.code != source.text())
            error = "Source file changed since parsing; reload to update line locations.";
        return true;
    }

} // namespace snt::view
