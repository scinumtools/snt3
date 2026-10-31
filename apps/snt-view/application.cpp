#include "application.h"

#include "browser_view.h"
#include "data_view.h"
#include "graph_view.h"
#include "inspector_view.h"
#include "notifications.h"
#include "source_view.h"
#include "source_graph_view.h"
#include "setup_view.h"
#include "viewer_model.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace snt::view {
    namespace {

        constexpr char VIEWER_TITLE[] = "SNT3 v" CODE_VERSION " -  Parameter Viewer";

        struct ExportDialogState {
            std::array<char, 4096> directory{};
            std::array<char, 256> filename{};
            std::string error;
        };

        struct SaveAsDialogState {
            std::array<char, 4096> parent{};
            std::array<char, 256> name{};
            std::string error;
        };

        struct NewSourceDialogState {
            std::array<char, 256> filename{};
            std::array<char, 256> name{};
            bool named = false;
            std::string error;
        };

        struct ConvertDialogState {
            std::string error;
        };

        void glfw_error(int code, const char* description) {
            std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
        }

        void trace_startup(const char* stage) {
            if (std::getenv("SNT_VIEW_TRACE")) {
                std::fprintf(stderr, "snt view startup: %s\n", stage);
                std::fflush(stderr);
            }
        }

        void open_save_as_dialog(
            const ViewerModel& model,
            SaveAsDialogState& state
        ) {
            const auto source_dir = model.artifact().parent_path();
            const std::string parent = source_dir.parent_path().string();
            const std::string name = source_dir.filename().string() + "-copy";
            std::snprintf(state.parent.data(), state.parent.size(), "%s", parent.c_str());
            std::snprintf(state.name.data(), state.name.size(), "%s", name.c_str());
            state.error.clear();
            ImGui::OpenPopup("Save Project As");
        }

        void open_export_dialog(const ViewerModel& model, ExportDialogState& state) {
            const auto project_directory = model.artifact().parent_path();
            const auto filename = project_directory.filename().string() + ".diph5";
            const auto directory = project_directory.string();
            std::snprintf(state.directory.data(), state.directory.size(), "%s", directory.c_str());
            std::snprintf(state.filename.data(), state.filename.size(), "%s", filename.c_str());
            state.error.clear();
            ImGui::OpenPopup("Export DIPH5");
        }

        void draw_directory_picker(const char* label, const char* child_id, std::array<char, 4096>& directory_text) {
            ImGui::PushID(label);
            ImGui::TextUnformatted(label);
            const auto& style = ImGui::GetStyle();
            const float up_width = ImGui::CalcTextSize("Up").x + 2.0f * style.FramePadding.x;
            ImGui::SetNextItemWidth(std::max(1.0f, ImGui::GetContentRegionAvail().x - up_width - style.ItemSpacing.x));
            ImGui::InputText("##directory", directory_text.data(), directory_text.size());
            ImGui::SameLine();
            const std::filesystem::path current_directory(directory_text.data());
            const auto parent = current_directory.parent_path();
            ImGui::BeginDisabled(parent.empty() || parent == current_directory);
            if (ImGui::Button("Up")) {
                const auto path = parent.string();
                std::snprintf(directory_text.data(), directory_text.size(), "%s", path.c_str());
            }
            ImGui::EndDisabled();
            std::vector<std::filesystem::path> directories;
            std::error_code directory_error;
            for (std::filesystem::directory_iterator it(
                     current_directory, std::filesystem::directory_options::skip_permission_denied, directory_error
                 ),
                 end;
                 !directory_error && it != end;
                 it.increment(directory_error)) {
                std::error_code type_error;
                if (it->is_directory(type_error) && !type_error)
                    directories.push_back(it->path());
            }
            std::sort(directories.begin(), directories.end());
            ImGui::BeginChild(child_id, ImVec2(560.0f, 230.0f), ImGuiChildFlags_Borders);
            for (const auto& directory : directories) {
                const auto name = directory.filename().string() + "/";
                if (ImGui::Selectable(name.c_str())) {
                    const auto path = directory.string();
                    std::snprintf(directory_text.data(), directory_text.size(), "%s", path.c_str());
                }
            }
            ImGui::EndChild();
            if (directory_error)
                ImGui::TextWrapped("Cannot list this directory: %s", directory_error.message().c_str());
            ImGui::PopID();
        }

        void draw_view(
            ViewerModel& model,
            InspectorCache& inspector_cache,
            DataViewState& data_state,
            GraphViewState& graph_state,
            SourceNavigationState& navigation,
            bool& request_save_as,
            SaveAsDialogState& save_as,
            NewSourceDialogState& new_source_state,
            ConvertDialogState& convert_state,
            ExportDialogState& export_state,
            SetupViewState& setup_state,
            NotificationCenter& notifications
        ) {
            auto& source = navigation.source;
            auto& source_owner = navigation.owner;
            auto& select_source_tab = navigation.select_tab;
            auto& source_error = navigation.error;
            auto& edit_report = navigation.edit_report;
            auto& save_as_parent = save_as.parent;
            auto& save_as_name = save_as.name;
            auto& save_as_error = save_as.error;
            const auto reload = [&] {
                if (source.dirty()) {
                    source_error = "Save or discard the edited document before reloading.";
                    return;
                }
                if (model.reload()) {
                    navigation.clear();
                }
            };
            bool request_export_diph5 = false;
            bool request_new_source = false;
            bool request_convert = false;
            bool created_source = false;
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Reload", "Ctrl+R"))
                        reload();
                    if (ImGui::MenuItem("New Source...", nullptr, false,
                                        model.can_save_as() && model.can_export_diph5())) {
                        if (source.dirty())
                            notifications.notify("Save or discard the edited document before creating a source.",
                                                 NoticeLevel::Warning);
                        else
                            request_new_source = true;
                    }
                    if (ImGui::MenuItem("Convert to Project...", nullptr, false, model.can_convert_to_project())) {
                        if (source.dirty())
                            notifications.notify("Save or discard the edited document before converting it to a project.",
                                                 NoticeLevel::Warning);
                        else
                            request_convert = true;
                    }
                    if (ImGui::MenuItem("Save As...", nullptr, false, model.can_save_as()))
                        request_save_as = true;
                    if (ImGui::MenuItem("Export DIPH5...", nullptr, false, model.can_export_diph5())) {
                        if (source.dirty())
                            notifications.notify("Save or discard the edited document before exporting DIPH5.",
                                                 NoticeLevel::Warning);
                        else
                            request_export_diph5 = true;
                    }
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
                if (model.object("@setup") && ImGui::MenuItem("Setup", nullptr, model.selection() == "@setup"))
                    model.select("@setup");
                ImGui::EndMainMenuBar();
            }
            const auto& io = ImGui::GetIO();
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false) && !io.WantTextInput)
                reload();
            if (request_save_as) {
                open_save_as_dialog(model, save_as);
                request_save_as = false;
            }
            if (request_new_source) {
                std::snprintf(new_source_state.filename.data(), new_source_state.filename.size(), "%s", "new-source.dip");
                std::snprintf(new_source_state.name.data(), new_source_state.name.size(), "%s", "new_source");
                new_source_state.named = false;
                new_source_state.error.clear();
                ImGui::OpenPopup("New Source");
            }
            if (request_convert) {
                convert_state.error.clear();
                ImGui::OpenPopup("Convert to Project");
            }
            if (request_export_diph5)
                open_export_dialog(model, export_state);

            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("New Source", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("Create a DIPL source in this project and add it to the DIPfile.");
                if (ImGui::RadioButton("Local code source", !new_source_state.named))
                    new_source_state.named = false;
                ImGui::SameLine();
                if (ImGui::RadioButton("Named source", new_source_state.named))
                    new_source_state.named = true;
                if (new_source_state.named) {
                    ImGui::TextUnformatted("Source name");
                    ImGui::SetNextItemWidth(420.0f);
                    ImGui::InputText("##new_source_name", new_source_state.name.data(), new_source_state.name.size());
                    ImGui::TextDisabled("Reference its values as {name?path.to.value}.");
                }
                ImGui::TextUnformatted("Source file name");
                ImGui::SetNextItemWidth(420.0f);
                ImGui::InputText("##new_source_filename", new_source_state.filename.data(), new_source_state.filename.size());
                ImGui::TextDisabled("Project directory: %s", model.artifact().parent_path().string().c_str());
                notifications.observe("new-source-error", new_source_state.error, NoticeLevel::Error);
                ImGui::Separator();
                ImGui::BeginDisabled(new_source_state.filename[0] == '\0' ||
                                     (new_source_state.named && new_source_state.name[0] == '\0'));
                if (ImGui::Button("Create Source")) {
                    const bool created = new_source_state.named
                        ? model.create_named_source(new_source_state.name.data(), new_source_state.filename.data())
                        : model.create_code_source(new_source_state.filename.data());
                    if (created) {
                        new_source_state.error.clear();
                        navigation.clear();
                        created_source = true;
                        notifications.notify("Created source: " + std::string(new_source_state.filename.data()));
                        ImGui::CloseCurrentPopup();
                    } else
                        new_source_state.error = model.error();
                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    new_source_state.error.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Convert to Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("Create a DIPfile that loads the current source. The source file stays in place.");
                ImGui::TextDisabled("Source: %s", model.artifact().string().c_str());
                ImGui::TextDisabled("Will create: %s", (model.artifact().parent_path() / "DIPfile").string().c_str());
                notifications.observe("convert-project-error", convert_state.error, NoticeLevel::Error);
                ImGui::Separator();
                if (ImGui::Button("Convert")) {
                    if (model.convert_to_project()) {
                        navigation.clear();
                        convert_state.error.clear();
                        notifications.notify("Opened project: " + model.artifact().string());
                        ImGui::CloseCurrentPopup();
                    } else
                        convert_state.error = model.error();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    convert_state.error.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::SetNextWindowPos(
                ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)
            );
            if (ImGui::BeginPopupModal("Save Project As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("Choose a parent directory and a name for the project copy.");
                draw_directory_picker("Parent directory", "Save As directories", save_as_parent);
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

            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Export DIPH5", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("Save the currently evaluated project as a DIPH5 snapshot. Commit setup and save source edits first.");
                draw_directory_picker("Destination directory", "Export directories", export_state.directory);
                ImGui::TextUnformatted("File name");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::InputText("##export_diph5_filename", export_state.filename.data(), export_state.filename.size());
                const auto destination = std::filesystem::path(export_state.directory.data()) / export_state.filename.data();
                ImGui::TextDisabled("Will create: %s", destination.string().c_str());
                notifications.observe("export-diph5-error", export_state.error, NoticeLevel::Error);
                ImGui::Separator();
                ImGui::BeginDisabled(export_state.filename[0] == '\0');
                if (ImGui::Button("Export")) {
                    if (source.dirty())
                        export_state.error = "Save or discard the edited document before exporting DIPH5.";
                    else if (std::filesystem::path(export_state.filename.data()).filename() != export_state.filename.data())
                        export_state.error = "Enter a file name without a directory path.";
                    else if (model.export_diph5(destination)) {
                        export_state.error.clear();
                        notifications.notify("Exported DIPH5: " + std::filesystem::absolute(destination).string());
                        ImGui::CloseCurrentPopup();
                    } else
                        export_state.error = model.error();
                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    export_state.error.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor();

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
                    data_state = {};
                    graph_state = {};
                    has_data = false;
                    has_graph = false;
                    if (navigation.open_selected(model)) {
                        if (created_source) {
                            source.set_editing(true);
                            select_source_tab = true;
                        }
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
                const auto* selected_object = model.object(model.selection());
                if (selected_object && selected_object->role == ObjectRole::Setup) {
                    draw_setup_view(model, setup_state, notifications);
                } else if (ImGui::BeginTabBar("Inspector tabs")) {
                    if (ImGui::BeginTabItem(
                            "Inspector",
                            nullptr,
                            selection_changed && !select_source_tab ? ImGuiTabItemFlags_SetSelected : 0
                        )) {
                        draw_inspector(model, inspector_cache, navigation, notifications);
                        ImGui::EndTabItem();
                    }
                    const bool selected_source =
                        selected_object &&
                        (selected_object->role == ObjectRole::Source || selected_object->role == ObjectRole::CodeSource ||
                         selected_object->role == ObjectRole::ProjectFile ||
                         selected_object->role == ObjectRole::BlockSource ||
                         selected_object->role == ObjectRole::DIPfileSource);
                    if (selected_source && !source.file().empty() && source_owner == model.selection() &&
                        ImGui::BeginTabItem("Source", nullptr, select_source_tab ? ImGuiTabItemFlags_SetSelected : 0)) {
                        draw_source_view(model, navigation, request_save_as, notifications);
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
                    if (selected_object && supports_source_graph(*selected_object) &&
                        ImGui::BeginTabItem("Sources graph")) {
                        draw_source_graph_view(model, navigation);
                        ImGui::EndTabItem();
                    }
                    if (displayed_selection == model.selection() && displayed_revision == model.revision())
                        select_source_tab = false;
                    ImGui::EndTabBar();
                }
                notifications.observe("source-navigation-error", source_error, NoticeLevel::Warning);
                ImGui::EndChild();
                ImGui::EndTable();
            }
            ImGui::End();
            notifications.observe("reload-error", model.error(), model.stale() ? NoticeLevel::Warning : NoticeLevel::Error);
        }

    } // namespace

    int run_application(ViewerModel& model) {
        trace_startup("model loaded");
        glfwSetErrorCallback(glfw_error);
#if defined(__linux__)
        // A virtual or regular X11 display should use the X11 backend even when
        // the release also supports Wayland.
        if (std::getenv("DISPLAY") && !std::getenv("WAYLAND_DISPLAY"))
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif
        if (!glfwInit()) {
            std::fprintf(stderr, "snt view: could not initialize a graphical display; check the desktop session and display server\n");
            return 1;
        }
        trace_startup("GLFW initialized");
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
            std::fprintf(stderr, "snt view: could not create an OpenGL window; check the display and graphics drivers\n");
            glfwTerminate();
            return 1;
        }
        trace_startup("window created");
        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
        trace_startup("OpenGL context current");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        trace_startup("ImGui context created");
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
        trace_startup("ImGui backends initialized");
        ImGui::StyleColorsLight();
        ImGuiStyle& style = ImGui::GetStyle();
        const float ui_scale = std::max(1.0f, ImGui_ImplGlfw_GetContentScaleForWindow(window));
        style.ScaleAllSizes(ui_scale);
        style.FontScaleDpi = ui_scale;
        style.FontSizeBase = 16.0f;
        io.Fonts->AddFontDefaultVector();
        trace_startup("font loaded");
        InspectorCache inspector_cache;
        DataViewState data_state;
        GraphViewState graph_state;
        SourceNavigationState navigation;
        bool request_save_as = false;
        SaveAsDialogState save_as;
        NewSourceDialogState new_source_state;
        ConvertDialogState convert_state;
        ExportDialogState export_state;
        SetupViewState setup_state;
        NotificationCenter notifications;
        bool first_frame = true;
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
                navigation,
                request_save_as,
                save_as,
                new_source_state,
                convert_state,
                export_state,
                setup_state,
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
            if (first_frame) {
                trace_startup("first frame rendered");
                first_frame = false;
            }
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }

} // namespace snt::view
