#include "pch_tests.h"

using namespace snt;

TEST(SolverTemplate, BasicParsing) {

  dip::DIP d;
  d.add_string("foo real = 23 cm");
  d.add_string("bar str = 'baz'");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  dip::ValueNode::PointerType qnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->to_string(), "23 cm");

  dip::TemplateSolver solver(env);
  std::string result = solver.eval("{{?foo}}, {{?bar}}");
  EXPECT_EQ(result, "23 cm, 'baz'");

}
