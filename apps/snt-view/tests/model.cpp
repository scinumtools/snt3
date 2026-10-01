#include "viewer_model.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void write(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream file(path);
    file << contents;
    if (!file) throw std::runtime_error("Unable to write test artifact");
}
} // namespace

int main() {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() / ("snt-view-model-" + suffix + ".dip");
    try {
        write(path, "a int = 1\nb int = 2\n");
        snt::view::ViewerModel model(path);
        check(model.revision() == 0, "Initial viewer revision is incorrect");
        check(model.object("a") && model.object("b"), "Values are missing from the browser");
        check(model.select("a") && model.select("b"), "Selection failed");
        check(model.back() && model.selection() == "a", "Back navigation failed");
        check(model.forward() && model.selection() == "b", "Forward navigation failed");
        model.set_search("a");

        write(path, "a int = 3\nb int = 4\n");
        check(model.reload() && model.selection() == "b" && model.search() == "a",
              "Reload did not preserve viewer state");
        check(model.revision() == 1, "Successful reload did not invalidate inspection data");
        check(model.environment().get_node("a")->value->to_string() == "3", "Reload kept the old value");

        write(path, "a int = invalid\n");
        check(!model.reload() && model.stale(), "Invalid reload should retain the last valid model");
        check(model.revision() == 1, "Failed reload invalidated the last valid inspection data");
        check(model.environment().get_node("a")->value->to_string() == "3", "Invalid reload changed the model");
        std::filesystem::remove(path);
        return 0;
    } catch (...) {
        std::filesystem::remove(path);
        throw;
    }
}
