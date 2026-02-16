#include "pch_tests.h"
#include <snt/dip/dip.h>

#include <iostream>
#include <limits>

using namespace snt;

TEST(ParseStrings, BlockQuotes) {

  dip::DIP d;
  d.add_string("foo str = \"\"\"");
  d.add_string("bar1");
  d.add_string("bar2");
  d.add_string("\"\"\"");
  d.add_string("bar str = \"\"\" \"\"\"");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->value_raw.at(0), "\nbar1\nbar2\n");
  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  EXPECT_EQ(node->value_raw.at(0), " ");
}

TEST(ParseStrings, EscapeSymbols) {

  dip::DIP d;
  d.add_string("foo str = \"\"\"foo \\\" foo \\\' foo \\n foo\"\"\"");
  d.add_string("bar str = \"bar \\\" bar \\\' bar \\n bar\"");
  d.add_string("baz str = 'baz \\\" baz \\\' baz \\n baz'");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 3);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->value_raw.at(0), "foo \\\" foo \\' foo \\n foo");
  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  EXPECT_EQ(node->value_raw.at(0), "bar \\\" bar \\' bar \\n bar");
  node = env.nodes.at(2);
  EXPECT_EQ(node->name, "baz");
  EXPECT_EQ(node->value_raw.at(0), "baz \\\" baz \\' baz \\n baz");
}

TEST(ParseStrings, CommentSymbol) {

  dip::DIP d;
  d.add_string("foo str = bar#baz # comment");
  d.add_string("bar str = \"baz#foo\" # comment");
  d.add_string("baz str[:] = [\"foo#\",\"bar\"] # comment");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 3);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->value_raw.at(0), "bar#baz");
  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  EXPECT_EQ(node->value_raw.at(0), "baz#foo");
  node = env.nodes.at(2);
  EXPECT_EQ(node->name, "baz");
  EXPECT_EQ(node->value_raw, val::Array::StringType({"foo#", "bar"}));
}
