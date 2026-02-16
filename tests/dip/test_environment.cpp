#include "pch_tests.h"
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

using namespace snt;

TEST(Environment, RequestValue) {

  dip::DIP d;
  d.add_string("foo bool = true");
  d.add_string("bar int = 3");
  dip::Environment env = d.parse();
  val::BaseValue::PointerType node = env.request_value("?bar");
  EXPECT_EQ(node->to_string(), "3");

}

TEST(Environment, RequestNodes) {

  dip::DIP d;
  d.add_string("foo.bar bool = true");
  d.add_string("foo.baz int = 3");
  dip::Environment env = d.parse();
  dip::ValueNode::ListType nodes = env.request_nodes("?foo");
  EXPECT_EQ(nodes.at(0)->to_string(), "true");
  EXPECT_EQ(nodes.at(1)->to_string(), "3");

}

//TODO: tests more requests

