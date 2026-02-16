#include <iostream>
#include <snt/exs/atom.h>
#include <snt/exs/solver.h>
#include <snt/val/values_array.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_value.h>

using namespace snt;

int main() {

  exs::Solver<exs::Atom> solver;
  exs::Atom atom = solver.solve("23 * 34.5 + 4");
  std::cout << atom.to_string() << '\n';

  val::ArrayValue<double> value({1.23, 4.56e7});
  std::cout << value.to_string() << '\n';

  puq::Quantity length("1*m");
  length = length.convert("km");
  std::cout << length.to_string() << '\n';

  dip::DIP d;
  d.add_string("foo int m");
  d.add_string("foo = 3 km");
  dip::Environment env = d.parse();
  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  std::cout << vnode->to_string() << '\n';
  std::cout << vnode->value->to_string() << '\n';
  std::cout << vnode->units->to_string() << '\n';
}
