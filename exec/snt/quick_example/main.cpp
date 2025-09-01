#include <iostream>

#include <exs.h>
#include <val.h>
#include <puq.h>
#include <dip.h>

using namespace snt;

int main() {

  exs::Solver<exs::Atom> solver;
  exs::Atom atom = solver.solve("23 * 34.5 + 4");
  std::cout << atom.to_string() << std::endl;

  val::ArrayValue<double> value({1.23, 4.56e7});
  std::cout << value.to_string() << std::endl;

  puq::Quantity length("1*m");
  length = length.convert("km");
  std::cout << length.to_string() << std::endl;

  dip::DIP d;
  d.add_string("foo int m");
  d.add_string("foo = 3 km");
  dip::Environment env = d.parse();
  dip::BaseNode::PointerType node = env.nodes.at(0);
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  std::cout << vnode->value->to_string() << std::endl;

}
