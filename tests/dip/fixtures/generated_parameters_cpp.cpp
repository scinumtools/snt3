#include "parameters.hpp"

#include <string_view>

int main() {
    const auto& values = parameters::parameters;
    if (values.experiment.title != std::string_view("Flow \"study\" \\ trial")) return 1;
    if (values.experiment.steps != 4) return 2;
    if (values.experiment.gains[0] != 1.25 || values.experiment.gains[2] != 3.75) return 3;
    if (values.experiment.paths[0] != "alpha" || values.experiment.paths[1] != "beta") return 4;
    if (values.sensors[0].id != 7 || !values.sensors[0].enabled) return 5;
    if (values.sensors[1].id != 9 || values.sensors[1].enabled) return 6;
    return 0;
}
