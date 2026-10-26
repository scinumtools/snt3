#include "application.h"

#include "browser_view.h"
#include "data_view.h"
#include "graph_view.h"
#include "inspector_view.h"
#include "notifications.h"
#include "source_view.h"
#include "viewer_model.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace snt::view {
    namespace {

        constexpr char VIEWER_TITLE[] = "SNT3 v" CODE_VERSION " -  Parameter Viewer";

        void glfw_error(int code, const char* description) {
            std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
        }

        void open_save_as_dialog(
            const ViewerModel& model,
            std::array<char, 4096>& parent_directory,
            std::array<char, 256>& project_name
        ) {
            const auto source_dir = model.artifact().parent_path();
            const std::string parent = source_dir.parent_path().string();
            const std::string name = source_dir.filename().string() + "-copy";
            std::snprintf(parent_directory.data(), parent_directory.size(), "%s", parent.c_str());
            std::snprintf(project_name.data(), project_name.size(), "%s", name.c_str());
            ImGui::OpenPopup("Save Project As");
        }

        void draw_view(
            ViewerModel& model,
            InspectorCache& inspector_cache,
            DataViewState& data_state,
            GraphViewState& graph_state,
            SourceView& source,
            std::string& source_owner,
            bool& select_source_tab,
            bool& scroll_to_target,
            std::string& source_error,
            std::string& edit_report,
            bool& request_save_as,
            std::array<char, 4096>& save_as_parent,
            std::array<char, 256>& save_as_name,
            std::string& save_as_error,
            NotificationCenter& notifications
        ) {
            const auto reload = [&] {
                if (source.dirty()) {
                    source_error = "Save or discard the edited document before reloading.";
                    return;
                }
                if (model.reload()) {
                    source = SourceView{};
                    source_owner.clear();
                    source_error.clear();
                    edit_report.clear();
                    select_source_tab = false;
                    scroll_to_target = false;
                }
            };
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Reload", "Ctrl+R"))
                        reload();
                    if (ImGui::MenuItem("Save As...", nullptr, false, model.can_save_as()))
                        open_save_as_dialog(model, save_as_parent, save_as_name);
                    if (ImGui::MenuItem("Close"))
                        glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Navigate")) {
                    if (ImGui::MenuItem("Back", nullptr, false, model.can_back()))
                        model.back();
                    if (ImGui::MenuItem("Forward", nullptr, false, model.can_forward()))
                        model.forward();
                    if (ImGui::MenuItem("Parent")) {
                        const auto* current = model.object(model.selection());
                        if (current)
                            model.select(current->parent);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            const auto& io = ImGui::GetIO();
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false) && !io.WantTextInput)
                reload();
            if (request_save_as) {
                open_save_as_dialog(model, save_as_parent, save_as_name);
                request_save_as = false;
            }

            ImGui::SetNextWindowPos(
                ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)
            );
            if (ImGui::BeginPopupModal("Save Project As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("Choose a parent directory and a name for the project copy.");
                ImGui::InputText("Parent directory", save_as_parent.data(), save_as_parent.size());
                ImGui::SameLine();
                const std::filesystem::path current_parent(save_as_parent.data());
                const auto parent_parent = current_parent.parent_path();
                ImGui::BeginDisabled(parent_parent.empty() || parent_parent == current_parent);
                if (ImGui::Button("Up")) {
                    const std::string path = parent_parent.string();
                    std::snprintf(save_as_parent.data(), save_as_parent.size(), "%s", path.c_str());
                }
                ImGui::EndDisabled();
                std::vector<std::filesystem::path> directories;
                std::error_code directory_error;
                for (std::filesystem::directory_iterator it(
                         current_parent, std::filesystem::directory_options::skip_permission_denied, directory_error
                     ),
                     end;
                     !directory_error && it != end;
                     it.increment(directory_error)) {
                    std::error_code type_error;
                    if (it->is_directory(type_error) && !type_error)
                        directories.push_back(it->path());
                }
                std::sort(directories.begin(), directories.end());
                ImGui::BeginChild("Save As directories", ImVec2(560.0f, 230.0f), ImGuiChildFlags_Borders);
                for (const auto& directory : directories) {
                    const auto label = directory.filename().string() + "/";
                    if (ImGui::Selectable(label.c_str())) {
                        const std::string path = directory.string();
                        std::snprintf(save_as_parent.data(), save_as_parent.size(), "%s", path.c_str());
                    }
                }
                ImGui::EndChild();
                if (directory_error)
                    ImGui::TextWrapped("Cannot list this directory: %s", directory_error.message().c_str());
                ImGui::TextUnformatted("New project folder name");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::InputText("##save_as_project_folder_name", save_as_name.data(), save_as_name.size());
                const auto destination_preview =
                    (std::filesystem::path(save_as_parent.data()) / save_as_name.data()).string();
                ImGui::TextDisabled("Will create: %s", destination_preview.c_str());
                notifications.observe("save-as-error", save_as_error, NoticeLevel::Error);
                ImGui::Separator();
                ImGui::BeginDisabled(save_as_name[0] == '\0');
                if (ImGui::Button("Save Copy")) {
                    const bool had_source = !source.file().empty();
                    const auto source_path = had_source ? model.editable_document_path(source.file()) : std::nullopt;
                    const auto source_text = source.text();
                    const auto source_format = source.format();
                    const auto source_line = source.source_line();
                    const bool was_editing = source.editing();
                    const auto edited_path = source.dirty() ? source_path : std::nullopt;
                    const auto destination = std::filesystem::path(save_as_parent.data()) / save_as_name.data();
                    if (model.save_as(destination, edited_path, source_text)) {
                        save_as_error.clear();
                        source_error.clear();
                        edit_report.clear();
                        if (source_path) {
                            const auto saved_file = model.artifact().parent_path() / *source_path;
                            source.open_document(
                                saved_file,
                                source_line,
                                model.document_text(*source_path),
                                source_format,
                                source_error
                            );
                            source.set_editing(was_editing);
                        } else
                            source = SourceView{};
                        ImGui::CloseCurrentPopup();
                    } else
                        save_as_error = model.error();
                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    save_as_error.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::Begin(
                "Viewer",
                nullptr,
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoSavedSettings
            );
            ImGui::TextUnformatted(model.input_path().c_str());
            ImGui::Separator();
            if (ImGui::BeginTable(
                    "ViewerPanels", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV, ImVec2(0.0f, -80.0f)
                )) {
                ImGui::TableSetupColumn("Browser", ImGuiTableColumnFlags_WidthStretch, 0.34f);
                ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch, 0.66f);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::BeginChild("Browser", ImVec2(0.0f, 0.0f));
                draw_browser(model);
                ImGui::EndChild();
                ImGui::TableSetColumnIndex(1);
                ImGui::BeginChild("Inspector", ImVec2(0.0f, 0.0f));
                static std::string displayed_selection;
                static std::size_t displayed_revision = model.revision();
                static bool has_data = false;
                static bool has_graph = false;
                const bool selection_changed =
                    displayed_selection != model.selection() || displayed_revision != model.revision();
                const bool preserve_source_tab = select_source_tab && displayed_selection == model.selection() &&
                                                 source_owner == model.selection() && !source.file().empty();
                if (selection_changed) {
                    source_error.clear();
                    if (!preserve_source_tab)
                        select_source_tab = false;
                    scroll_to_target = false;
                    data_state = {};
                    graph_state = {};
                    has_data = false;
                    has_graph = false;
                    if (open_selected_source(model, source, source_error)) {
                        source_owner = model.selection();
                    }
                    if (preserve_source_tab)
                        select_source_tab = true;
                    if (const auto* selected = model.object(model.selection()); selected) {
                        try {
                            const std::string inspect_path = model.data_path(*selected);
                            if (inspect_path.empty())
                                throw std::out_of_range("No data path for the selected source");
                            const auto capabilities = dip::Inspector{model.environment()}.capabilities(inspect_path);
                            has_data = capabilities.hasArrayData || capabilities.hasTabularData;
                            has_graph = capabilities.hasReferenceGraph;
                        } catch (const std::out_of_range&) {
                            // Browser-only structural entries do not have inspection capabilities.
                        }
                    }
                }
                displayed_selection = model.selection();
                displayed_revision = model.revision();
                if (ImGui::BeginTabBar("Inspector tabs")) {
                    if (ImGui::BeginTabItem(
                            "Inspector",
                            nullptr,
                            selection_changed && !select_source_tab ? ImGuiTabItemFlags_SetSelected : 0
                        )) {
                        draw_inspector(
                            model,
                            inspector_cache,
                            source,
                            select_source_tab,
                            scroll_to_target,
                            source_error,
                            notifications
                        );
                        ImGui::EndTabItem();
                    }
                    const auto* selected_object = model.object(model.selection());
                    const bool selected_source =
                        selected_object &&
                        (selected_object->role == ObjectRole::Source || selected_object->role == ObjectRole::CodeSource ||
                         selected_object->role == ObjectRole::BlockSource ||
                         selected_object->role == ObjectRole::DIPfileSource);
                    if (selected_source && !source.file().empty() && source_owner == model.selection() &&
                        ImGui::BeginTabItem("Source", nullptr, select_source_tab ? ImGuiTabItemFlags_SetSelected : 0)) {
                        draw_source_view(model, source, scroll_to_target, source_error, edit_report,
                                         request_save_as, select_source_tab, notifications);
                        ImGui::EndTabItem();
                    }
                    if (has_data && ImGui::BeginTabItem("Data")) {
                        draw_data_view(model, data_state);
                        ImGui::EndTabItem();
                    }
                    if (has_graph && ImGui::BeginTabItem("Graph")) {
                        draw_graph_view(model, graph_state);
                        ImGui::EndTabItem();
                    }
                    if (displayed_selection == model.selection() && displayed_revision == model.revision())
                        select_source_tab = false;
                    ImGui::EndTabBar();
                }
                ImGui::EndChild();
                ImGui::EndTable();
            }
            ImGui::End();
            notifications.observe("reload-error", model.error(), model.stale() ? NoticeLevel::Warning : NoticeLevel::Error);
        }

    } // namespace

    int run_application(ViewerModel& model) {
        glfwSetErrorCallback(glfw_error);
        if (!glfwInit())
            return 1;
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
        DataViewState data_state;
        GraphViewState graph_state;
        SourceView source;
        std::string source_owner;
        bool select_source_tab = false;
        bool scroll_to_target = false;
        std::string source_error;
        std::string edit_report;
        bool request_save_as = false;
        std::array<char, 4096> save_as_parent{};
        std::array<char, 256> save_as_name{};
        std::string save_as_error;
        NotificationCenter notifications;
        while (!glfwWindowShouldClose(window)) {
            // Input wakes the frame immediately; the timeout keeps idle UI updates active.
            glfwWaitEventsTimeout(1.0 / 60.0);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            draw_view(
                model,
                inspector_cache,
                data_state,
                graph_state,
                source,
                source_owner,
                select_source_tab,
                scroll_to_target,
                source_error,
                edit_report,
                request_save_as,
                save_as_parent,
                save_as_name,
                save_as_error,
                notifications
            );
            notifications.draw();
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
