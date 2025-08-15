#include <gtest/gtest.h>

#include "../../src/dip/dip.h"
#include "../../src/dip/environment.h"
#include "../../src/dip/nodes/nodes.h"

TEST(Branchig, FirstCase) {

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

TEST(Branchig, SecondCase) {

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

TEST(Branchig, FirstAndSecondCase) {

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

TEST(Branchig, ElseCase) {

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

TEST(Branchig, ConsecutiveBranches) {

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

TEST(Branchig, LowerIndentClosing) {

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

TEST(Branchig, NestedBranches) {

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
