#include "pch_tests.h"

#include <snt/dip/dip.h>

using namespace snt;

TEST(Hierarchy, ParentChildNodes) {

  dip::DIP d;
  d.add_string("foo int = 2\n  bar int = 3\n  bim int = 4\nbom int = 5");
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

TEST(Hierarchy, IndentationChecking) {

  dip::DIP d;
  d.add_string("foo int = 2\n bar int = 3");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Indent of the current node is not a multiple of 2 '1':  bar int = 3");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }

  d = dip::DIP();
  d.add_string("foo int = 2\n    bar int = 3");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Indent of the child node '4' is not exactly 2 white spaces higher than its parent nodes '4':     bar int = 3");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}
