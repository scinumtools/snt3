#include "setup_view.h"

#include "notifications.h"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <optional>
#include <stdexcept>

namespace snt::view {
    namespace {
        int resize_text(ImGuiInputTextCallbackData* data) {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                auto* value = static_cast<std::string*>(data->UserData);
                value->resize(static_cast<std::size_t>(data->BufTextLen));
                data->Buf = value->data();
            }
            return 0;
        }

        float input_width(const float gap) {
            const float right_margin = ImGui::GetStyle().ScrollbarSize + gap;
            return std::max(1.0f, ImGui::GetContentRegionAvail().x - right_margin);
        }

        bool text_input(const char* label, std::string& value, bool multiline, const float width) {
            value.reserve(std::max<std::size_t>(value.capacity(), 256));
            const auto flags = ImGuiInputTextFlags_CallbackResize;
            if (!multiline)
                ImGui::SetNextItemWidth(width);
            const bool changed = multiline
                                     ? ImGui::InputTextMultiline(label, value.data(), value.capacity() + 1,
                                                                 ImVec2(width, 72.0f), flags, resize_text, &value)
                                     : ImGui::InputText(label, value.data(), value.capacity() + 1, flags,
                                                        resize_text, &value);
            value.resize(std::strlen(value.c_str()));
            return changed;
        }
    } // namespace

    void draw_setup_view(ViewerModel& model, SetupViewState& state, NotificationCenter& notifications) {
        if (state.revision != model.revision()) {
            const std::string output_name = state.output_name;
            state = {};
            state.output_name = output_name;
            state.revision = model.revision();
        }
        const float base_size = ImGui::GetStyle().FontSizeBase;
        const float gap = ImGui::GetFontSize() * 0.45f;
        const float inset = ImGui::GetFontSize() * 0.7f;
        const ImVec4 heading_color(0.18f, 0.28f, 0.39f, 1.0f);
        const ImVec4 secondary_color = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
        const ImVec4 error_color(0.68f, 0.20f, 0.16f, 1.0f);
        const ImVec4 error_background(1.0f, 0.94f, 0.92f, 1.0f);

        ImGui::Dummy(ImVec2(0.0f, gap));
        ImGui::Indent(inset);
        ImGui::PushFont(nullptr, base_size * 1.22f);
        ImGui::PushStyleColor(ImGuiCol_Text, heading_color);
        ImGui::TextUnformatted("Guided setup");
        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::PushStyleColor(ImGuiCol_Text, secondary_color);
        ImGui::TextWrapped("Set the project parameters marked with !setup. Answers are validated together when you commit.");
        ImGui::PopStyleColor();
        ImGui::Unindent(inset);
        ImGui::Dummy(ImVec2(0.0f, gap));
        ImGui::Separator();
        for (const auto& question : model.setup_questions()) {
            ImGui::PushID(question.path.c_str());
            const bool invalid = state.error_path == question.path;
            const bool focus_error = invalid && state.scroll_to_error;
            ImGui::Dummy(ImVec2(0.0f, gap));
            ImGui::Indent(inset);
            ImGui::PushFont(nullptr, base_size * 1.10f);
            ImGui::PushStyleColor(ImGuiCol_Text, invalid ? error_color : heading_color);
            ImGui::TextUnformatted(question.path.c_str());
            ImGui::PopStyleColor();
            ImGui::PopFont();
            if (!question.description.empty()) {
                ImGui::PushFont(nullptr, base_size * 0.93f);
                ImGui::PushStyleColor(ImGuiCol_Text, secondary_color);
                ImGui::TextWrapped("%s", question.description.c_str());
                ImGui::PopStyleColor();
                ImGui::PopFont();
            }
            ImGui::Dummy(ImVec2(0.0f, gap * 0.35f));
            if (question.has_value)
                ImGui::TextDisabled("Current: %s", question.current_value.c_str());
            else
                ImGui::TextDisabled("Required: no current value");
            ImGui::Dummy(ImVec2(0.0f, gap * 0.35f));
            if (invalid) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, error_background);
                ImGui::PushStyleColor(ImGuiCol_Border, error_color);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
            }
            if (focus_error)
                ImGui::SetKeyboardFocusHere();

            if (!question.options.empty()) {
                const auto found = state.choices.find(question.path);
                const std::optional<std::size_t> selected = found == state.choices.end()
                                                                ? std::nullopt : std::make_optional(found->second);
                const char* preview = !selected
                                          ? (question.has_value ? "Keep current" : "Select an option")
                                          : question.options[*selected].label.c_str();
                float label_width = ImGui::CalcTextSize(preview).x;
                for (const auto& option : question.options)
                    label_width = std::max(label_width, ImGui::CalcTextSize(option.label.c_str()).x);
                const float desired_width = label_width + ImGui::GetFrameHeight() +
                                            2.0f * ImGui::GetStyle().FramePadding.x + gap;
                ImGui::SetNextItemWidth(std::min(desired_width, input_width(gap)));
                if (ImGui::BeginCombo("##value", preview)) {
                    if (question.has_value && ImGui::Selectable("Keep current", !selected)) {
                        state.choices.erase(question.path);
                        state.error_path.clear();
                    }
                    for (std::size_t index = 0; index < question.options.size(); ++index)
                        if (ImGui::Selectable(question.options[index].label.c_str(),
                                              selected && *selected == index)) {
                            state.choices[question.path] = index;
                            state.error_path.clear();
                        }
                    ImGui::EndCombo();
                }
            } else if (question.type == dip::NodeDtype::Boolean && !question.array) {
                const auto found = state.answers.find(question.path);
                const std::string current = question.has_value ? question.current_value : std::string{};
                std::string selected = found != state.answers.end() ? found->second : current;
                if (ImGui::RadioButton("True", selected == "true")) {
                    selected = "true";
                    if (current == "true")
                        state.answers.erase(question.path);
                    else
                        state.answers[question.path] = "true";
                    state.error_path.clear();
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("False", selected == "false")) {
                    selected = "false";
                    if (current == "false")
                        state.answers.erase(question.path);
                    else
                        state.answers[question.path] = "false";
                    state.error_path.clear();
                }
            } else {
                auto& value = state.text[question.path];
                if (text_input("##value", value, question.array, input_width(gap))) {
                    state.answers[question.path] = value;
                    state.error_path.clear();
                }
                if (question.has_value && state.answers.count(question.path) && ImGui::SmallButton("Keep current")) {
                    state.answers.erase(question.path);
                    value.clear();
                    state.error_path.clear();
                }
                if (question.type == dip::NodeDtype::String && !question.array && !question.has_value &&
                    !state.answers.count(question.path) && ImGui::SmallButton("Set empty text")) {
                    state.answers[question.path] = "";
                    state.error_path.clear();
                }
            }
            if (focus_error) {
                ImGui::SetScrollHereY(0.25f);
                state.scroll_to_error = false;
            }
            if (invalid) {
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(2);
                if (state.error_path == question.path)
                    ImGui::TextColored(error_color, "Please correct this value");
            }
            ImGui::Unindent(inset);
            ImGui::Dummy(ImVec2(0.0f, gap));
            ImGui::Separator();
            ImGui::PopID();
        }
        ImGui::Dummy(ImVec2(0.0f, gap));
        ImGui::Indent(inset);
        const bool invalid_filename = state.error_path == "@setup-file";
        ImGui::PushFont(nullptr, base_size * 1.10f);
        ImGui::PushStyleColor(ImGuiCol_Text, invalid_filename ? error_color : heading_color);
        ImGui::TextUnformatted("Setup file name");
        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(0.0f, gap * 0.7f));
        if (invalid_filename) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, error_background);
            ImGui::PushStyleColor(ImGuiCol_Border, error_color);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        }
        if (invalid_filename && state.scroll_to_error)
            ImGui::SetKeyboardFocusHere();
        if (text_input("##setup-output", state.output_name, false, input_width(gap)))
            state.error_path.clear();
        if (invalid_filename) {
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }
        if (invalid_filename && state.scroll_to_error) {
            ImGui::SetScrollHereY(0.75f);
            state.scroll_to_error = false;
        }
        ImGui::Dummy(ImVec2(0.0f, gap));
        if (ImGui::Button("Commit setup")) {
            if (model.commit_setup(state.answers, state.choices, state.output_name))
                notifications.notify("Setup committed and project reloaded.");
            else {
                state.error_path = model.setup_error_path();
                state.scroll_to_error = !state.error_path.empty();
                notifications.notify(model.error(), NoticeLevel::Error);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove override")) {
            if (model.remove_setup_override(state.output_name))
                notifications.notify("Guided setup override removed and project reloaded.");
            else
                notifications.notify(model.error(), NoticeLevel::Error);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Remove the registered override named in Setup file name.");
        ImGui::Unindent(inset);
    }
} // namespace snt::view
