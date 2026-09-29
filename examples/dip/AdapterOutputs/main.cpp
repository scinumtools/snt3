#include <cstdint>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <snt/dip/adapter.h>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class SolverAdapter final : public snt::dip::Adapter {
  public:
    void plan(const snt::dip::Environment& env, snt::dip::AdapterContext& context) const override {
        const auto steps = env["run.steps"].as<std::int64_t>();
        const auto dt = env["run.dt"].as<double>();
        context.add_text(
            "solver/control.nml",
            "&run\n  steps = " + std::to_string(steps) + "\n  dt_seconds = " + std::to_string(dt) + "\n/\n"
        );
        context.add_binary("solver/magic.bin", {0x53, 0x4e, 0x54, 0x33});
        context.add_stream("solver/times.dat", [steps, dt](std::ostream& out) {
            for (std::int64_t i = 0; i < steps; ++i)
                out << i * dt << '\n';
        });
    }
};

int main(int argc, char* argv[]) {
    const fs::path project = argc > 1 ? argv[1] : "examples/dip/AdapterOutputs/DIPfile";
    const fs::path output = argc > 2 ? argv[2] : "build/adapter-cpp";
    const auto files = snt::dip::run_adapter_project(project, SolverAdapter{}, output, "run.diph5");
    for (const auto& file : files)
        std::cout << file << '\n';
}
