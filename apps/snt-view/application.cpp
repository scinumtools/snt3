#include "application.h"

#include "browser_view.h"
#include "data_view.h"
#include "graph_view.h"
#include "inspector_view.h"
#include "source_view.h"
#include "viewer_model.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <string>

namespace snt::view {
    namespace {

        constexpr char VIEWER_TITLE[] = "SNT3 v" CODE_VERSION " -  Parameter Viewer";

        void glfw_error(int code, const char* description) {
            std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
        }

        void draw_view(
            ViewerModel& model,
            InspectorCache& inspector_cache,
            DataViewState& data_state,
            GraphViewState& graph_state,
            SourceView& source,
            bool& select_source_tab,
            bool& scroll_to_target,
            std::string& source_error
        ) {
            const auto reload = [&] {
                if (model.reload()) {
                    source = SourceView{};
                    source_error.clear();
                    select_source_tab = false;
                    scroll_to_target = false;
                }
            };
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Reload", "Ctrl+R"))
                        reload();
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
            if (model.stale()) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.2f, 1.0f), "Last valid view — reload failed");
            }
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
                const bool selection_changed = displayed_selection != model.selection() ||
                                               displayed_revision != model.revision();
                if (selection_changed) {
                    source = SourceView{};
                    source_error.clear();
                    select_source_tab = false;
                    scroll_to_target = false;
                    data_state = {};
                    graph_state = {};
                    has_data = false;
                    has_graph = false;
                    if (const auto* selected = model.object(model.selection()); selected &&
                        (selected->role == ObjectRole::Path || selected->role == ObjectRole::Override ||
                         selected->role == ObjectRole::BlockSource || selected->role == ObjectRole::Source) &&
                        (!selected->node_path.empty() || selected->role == ObjectRole::Source)) {
                        try {
                            const std::string inspect_path = selected->source_name.empty()
                                ? selected->node_path : selected->path;
                            const auto capabilities = dip::inspect_capabilities(model.environment(), inspect_path);
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
                            "Inspector", nullptr, selection_changed ? ImGuiTabItemFlags_SetSelected : 0
                        )) {
                        draw_inspector(
                            model, inspector_cache, source, select_source_tab, scroll_to_target, source_error
                        );
                        ImGui::EndTabItem();
                    }
                    if (!source.file().empty() &&
                        ImGui::BeginTabItem("Source", nullptr, select_source_tab ? ImGuiTabItemFlags_SetSelected : 0)) {
                        draw_source_view(model, source, scroll_to_target, source_error);
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
                    select_source_tab = false;
                    ImGui::EndTabBar();
                }
                ImGui::EndChild();
                ImGui::EndTable();
            }
            ImGui::SeparatorText("Diagnostics");
            if (model.error().empty())
                ImGui::TextDisabled("No reload error.");
            else
                ImGui::TextWrapped("%s", model.error().c_str());
            ImGui::End();
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
        bool select_source_tab = false;
        bool scroll_to_target = false;
        std::string source_error;
        while (!glfwWindowShouldClose(window)) {
            // Input wakes the frame immediately; the timeout keeps idle UI updates active.
            glfwWaitEventsTimeout(1.0 / 60.0);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            draw_view(model, inspector_cache, data_state, graph_state, source,
                      select_source_tab, scroll_to_target, source_error);
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
