#include <gtest/gtest.h>

#include "dip/dip.h"
#include "dip/environment.h"
#include "dip/nodes/nodes.h"

TEST(Hierarchy, ParentChildNodes) {

  dip::DIP d;
  d.add_string("foo int = 2\n  bar int = 3\n bim int = 4\nbom int = 5");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "foo.bar");

  node = env.nodes.at(2);
  EXPECT_EQ(node->name, "foo.bim");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "bom");
}

TEST(Hierarchy, GroupNodes) {

  dip::DIP d;
  d.add_string("foo\n  bar int = 3");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo.bar");
}
