#include "application.h"
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
#include <string>
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

void draw_tree(ViewerModel& model, const ObjectInfo& object, const std::string& query) {
    for (const auto& path : object.children) {
        const auto* item = model.object(path);
        if (!item || !matches(model, *item, query)) continue;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (item->children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (model.selection() == path) flags |= ImGuiTreeNodeFlags_Selected;
        if (!query.empty()) flags |= ImGuiTreeNodeFlags_DefaultOpen;
        const bool open = ImGui::TreeNodeEx(item->path.c_str(), flags, "%s", item->label.c_str());
        if (ImGui::IsItemClicked()) model.select(path);
        if (open && !item->children.empty()) {
            draw_tree(model, *item, query);
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
    std::string error;
};

void refresh_inspector(ViewerModel& model, InspectorCache& cache) {
    cache = {};
    cache.path = model.selection();
    cache.revision = model.revision();
    cache.valid = true;
    const auto* object = model.object(model.selection());
    if (!object) return;
    cache.fields.emplace_back("Path", object->path.empty() ? model.artifact().string() : object->path);
    if (!object->has_value) {
        cache.fields.emplace_back("Kind", object->path.empty() ? "Artifact" : "Group");
        return;
    }
    try {
        const auto node = model.environment().get_node(object->path);
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

        const auto provenance = model.environment()[object->path].get_provenance();
        if (!provenance.source_name.empty()) {
            const std::string source = provenance.source && !provenance.source->path.empty()
                ? provenance.source->path : provenance.source_name;
            cache.fields.emplace_back("Declared", source + ":" + std::to_string(provenance.source_line));
        }
        if (provenance.override_line) {
            const std::string source = provenance.override_source && !provenance.override_source->path.empty()
                ? provenance.override_source->path : provenance.source_name;
            cache.fields.emplace_back("Override", source + ":" + std::to_string(provenance.override_line));
        }
        const auto schemas = model.environment().get_applied_schemas(object->path);
        if (!schemas.empty()) {
            std::string names;
            for (const auto& schema : schemas) {
                if (!names.empty()) names += ", ";
                names += schema.name;
            }
            cache.fields.emplace_back("Schemas", std::move(names));
        }

        const auto& graph = model.environment().dependency_graph();
        if (graph.recorded) {
            const std::string graph_path = "?" + object->path;
            for (const auto& read : graph.dependencies(graph_path)) cache.reads.push_back(read.target);
            cache.readers = graph.referenced_by(graph_path);
        }
    } catch (const std::exception& error) {
        cache.error = error.what();
    }
}

void draw_inspector(ViewerModel& model, InspectorCache& cache) {
    if (!cache.valid || cache.path != model.selection() || cache.revision != model.revision())
        refresh_inspector(model, cache);
    for (const auto& [label, value] : cache.fields) labeled_text(label, value);
    if (!cache.reads.empty()) {
        ImGui::SeparatorText("Depends on");
        for (const auto& read : cache.reads) {
            const std::string target = read.size() > 1 && read.front() == '?' ? read.substr(1) : std::string{};
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
                ? reader.substr(1) : std::string{};
            if (!target.empty() && model.object(target)) {
                if (ImGui::Selectable(reader.c_str())) model.select(target);
            } else {
                ImGui::TextUnformatted(reader.c_str());
            }
        }
    }
    if (!cache.error.empty()) ImGui::TextWrapped("Inspection failed: %s", cache.error.c_str());
}

void draw_view(ViewerModel& model, InspectorCache& inspector_cache) {
    static char search[256] = {};
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Reload", "Ctrl+R")) model.reload();
            if (ImGui::MenuItem("Close")) glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Navigate")) {
            if (ImGui::MenuItem("Back")) model.back();
            if (ImGui::MenuItem("Forward")) model.forward();
            if (ImGui::MenuItem("Parent")) {
                const auto* current = model.object(model.selection());
                if (current) model.select(current->parent);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    const auto& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false) && !io.WantTextInput) model.reload();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("Viewer", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextUnformatted(model.artifact().string().c_str());
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
        if (ImGui::InputTextWithHint("##search", "Search paths", search, sizeof(search)))
            model.set_search(search);
        if (const auto* root = model.object("")) draw_tree(model, *root, model.search());
        ImGui::EndChild();
        ImGui::TableSetColumnIndex(1);
        ImGui::BeginChild("Inspector", ImVec2(0.0f, 0.0f));
        ImGui::TextUnformatted("Inspector");
        ImGui::Separator();
        draw_inspector(model, inspector_cache);
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
    while (!glfwWindowShouldClose(window)) {
        // Input wakes the frame immediately; the timeout keeps idle UI updates active.
        glfwWaitEventsTimeout(1.0 / 60.0);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        draw_view(model, inspector_cache);
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
