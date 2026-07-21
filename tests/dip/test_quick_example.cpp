#include "pch_tests.h"

#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

using namespace snt;

constexpr std::string_view SOURCE_ROOT{PROJECT_SOURCE_ROOT_DIR};
constexpr std::string_view PATH_EXAMPLE_DIP = "examples/dipl/";

TEST(QuickExample, ParseProperties) {

    std::string full_path = std::string(SOURCE_ROOT) + std::string(PATH_EXAMPLE_DIP) + "parameters.dip";

    dip::DIP d;
    d.add_file(full_path);
    dip::Environment env = d.parse();
    double density = env["simulation.fluid.density"].as<double>();
    int64_t steps = env["simulation.time.steps"].as<int64_t>();
    EXPECT_EQ(density, 998.2);
    EXPECT_EQ(steps, 10000);
}
