#include <iostream>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/puq/quantity.h>

using namespace snt;

constexpr std::string_view PATH_EXAMPLE_DIP = "examples/snt/quick_example/";

int main() {

    puq::Quantity length("1*m");
    length = length.convert("km");
    std::cout << length.to_string() << '\n';

    dip::DIP d;
    d.add_file(std::string(PATH_EXAMPLE_DIP) + "parameters.dip");
    dip::Environment env = d.parse();
    dip::Cursor cursor = env["simulation.fluid.density"];
    std::cout << cursor.to_string() << '\n';
}
