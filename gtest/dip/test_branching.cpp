#include "pch_tests.h"
#include "fixtures.h"

using namespace snt;

class Branching : public FixtureFunctions {};

TEST_F(Branching, FirstCase) {

  dip::DIP d;
  d.add_string("@case true");
  d.add_string("  age int = 30");
  d.add_string("@case false");
  d.add_string("  height float = 180");
  d.add_string("@else");
  d.add_string("  weight float = 80");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);
  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "age");
}

TEST_F(Branching, SecondCase) {

  // second case
  dip::DIP d;
  d.add_string("@case false");
  d.add_string("  age int = 30");
  d.add_string("@case true");
  d.add_string("  height float = 180");
  d.add_string("@else");
  d.add_string("  weight float = 80");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);
  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "height");
}

TEST_F(Branching, FirstAndSecondCase) {

  // first case
  dip::DIP d;
  d.add_string("@case true");
  d.add_string("  age int = 30");
  d.add_string("@case true");
  d.add_string("  height float = 180");
  d.add_string("@else");
  d.add_string("  weight float = 80");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);
  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "age");
}

TEST_F(Branching, ElseCase) {

  dip::DIP d;
  d.add_string("@case false");
  d.add_string("  age int = 30");
  d.add_string("@case false");
  d.add_string("  height float = 180");
  d.add_string("@else");
  d.add_string("  weight float = 80");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);
  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "weight");
}

TEST_F(Branching, ConsecutiveBranches) {

  dip::DIP d;
  d.add_string("@case true");
  d.add_string("  age int = 30");
  d.add_string("@else");
  d.add_string("  weight float = 80");
  d.add_string("@end");
  d.add_string("@case false");
  d.add_string("  age int = 30");
  d.add_string("@else");
  d.add_string("  weight float = 80");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);
  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "age");
  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "weight");
}

TEST_F(Branching, LowerIndentClosing) {

  dip::DIP d;
  d.add_string("man");
  d.add_string("  boy");
  d.add_string("    @case true");
  d.add_string("      age int = 15");
  d.add_string("    @else");
  d.add_string("      weight float = 40");
  d.add_string("  @case true");
  d.add_string("    age int = 30");
  d.add_string("  @else");
  d.add_string("    weight float = 80");
  d.add_string("@case false");
  d.add_string("  age int = 30");
  d.add_string("@else");
  d.add_string("  weight float = 80");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 3);
  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "man.boy.age");
  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "man.age");
  node = env.nodes.at(2);
  EXPECT_EQ(node->name, "weight");
}

TEST_F(Branching, NestedBranches) {

  dip::DIP d;
  d.add_string("man");
  d.add_string("  @case true");
  d.add_string("    @case true");
  d.add_string("      age int = 30"); // this is taken
  d.add_string("    @else");
  d.add_string("      weight float = 80");
  d.add_string("  @else");
  d.add_string("    weight float = 40");
  d.add_string("  @end");
  d.add_string("@case false");
  d.add_string("  @case true");
  d.add_string("    age int = 30"); // this should be ignored
  d.add_string("  @else");
  d.add_string("    weight float = 80");
  d.add_string("@else");
  d.add_string("  @case true");
  d.add_string("    weight float = 40"); // this is taken
  d.add_string("  @else");
  d.add_string("    human bool = true");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);
  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "man.age");
  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "weight");
}

TEST_F(Branching, ValueFunction) {

  dip::DIP d;
  d.add_value_function("scalar_value", FixtureFunctions::get_scalar_boolean);
  d.add_string("@case (scalar_value)");
  d.add_string("  age int = 30");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);
  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "age");
}

TEST_F(Branching, ValueInjection) {

  dip::DIP d;
  d.add_string("foo bool = true");
  d.add_string("@case {?foo}");
  d.add_string("  age int = 30");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);
  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "age");
}

TEST_F(Branching, ValueExpression) {

  dip::DIP d;
  d.add_string("foo int = 34");
  d.add_string("@case ('{?foo} <= 50')");
  d.add_string("  age int = 30");
  d.add_string("@end");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);
  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "age");
}
