#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "dip/dip.h"
#include "dip/environment.h"
#include "dip/nodes/nodes.h"

TEST(References, BooleanValues) {

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("foo bool = true");
  d.add_string("bar bool = {?foo}");
  d.add_string("snap bool[2] = [true, false]");
  d.add_string("crackle bool[2] = {?snap}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 4);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "true");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "crackle");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(References, IntegerValues) {

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("foo int = 32");
  d.add_string("bar int = {?foo}");
  d.add_string("snap int[2] = [32, 54]");
  d.add_string("crackle int[2] = {?snap}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 4);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "32");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "crackle");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[32, 54]");
}

TEST(References, FloatValues) {

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("foo float = 2.345e6");
  d.add_string("bar float = {?foo}");
  d.add_string("snap float[2] = [2.345e6, 3.456e7]");
  d.add_string("crackle float[2] = {?snap}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 4);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "2.3450e+06");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "crackle");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[2.3450e+06, 3.4560e+07]");
}

TEST(References, StringValues) {

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("foo str = 'baz'");
  d.add_string("bar str = {?foo}");
  d.add_string("snap str[2] = ['pop','jerk']");
  d.add_string("crackle str[2] = {?snap}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 4);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "'baz'");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "crackle");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "['pop', 'jerk']");
}

TEST(References, ExceptionMissingNode) {

  dip::DIP d;
  d.add_string("bar str = {?foo}");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Value environment request returns an empty pointer: ?foo");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(References, ExceptionDataType) {

  dip::DIP d;
  d.add_string("foo bool = true");
  d.add_string("bar str = {?foo}");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Assigning 'bool' value to the 'str' node: bar str = {?foo}");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(References, ExceptionDimension) {

  dip::DIP d;
  d.add_string("foo int[2] = [1,2]");
  d.add_string("bar int = {?foo}");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Assigning array value to the scalar node: bar int = {?foo}");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(References, TableNodes) {

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("foo");
  d.add_string("  snap str = 'snap'");
  d.add_string("  crackle bool[2] = [true,false]");
  d.add_string("bar table = {?foo}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 4);

  dip::BaseNode::PointerType node = env.nodes.at(2);
  EXPECT_EQ(node->name, "bar.snap");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "'snap'");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "bar.crackle");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(References, ImportNodes) {

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("foo");
  d.add_string("  snap str = 'snap'");
  d.add_string("  crackle bool[2] = [true,false]");
  d.add_string("bar {?}");
  d.add_string("baz");
  d.add_string("  {?foo}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 6);

  dip::BaseNode::PointerType node = env.nodes.at(2);
  EXPECT_EQ(node->name, "bar.foo.snap");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "'snap'");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "bar.foo.crackle");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[true, false]");

  node = env.nodes.at(4);
  EXPECT_EQ(node->name, "baz.snap");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "'snap'");

  node = env.nodes.at(5);
  EXPECT_EQ(node->name, "baz.crackle");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(References, RemoteSource) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_source.dip";
  std::string source_code = "foo\n  bar int = 3\n  baz bool[2] = [false,true]";
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code;
  }

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("$source " + source_name + " = " + source_filename.string());
  d.add_string("snap int = {" + source_name + "?foo.bar}");
  d.add_string("crackle bool[2] = {" + source_name + "?foo.baz}");
  d.add_string("pop {" + source_name + "?foo}");
  d.add_string("jerk {" + source_name + "?}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 6);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "snap");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "3");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "crackle");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[false, true]");

  node = env.nodes.at(2);
  EXPECT_EQ(node->name, "pop.bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "3");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "pop.baz");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[false, true]");

  node = env.nodes.at(4);
  EXPECT_EQ(node->name, "jerk.foo.bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "3");

  node = env.nodes.at(5);
  EXPECT_EQ(node->name, "jerk.foo.baz");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[false, true]");
}

TEST(References, ExceptionSource) {

  dip::DIP d;
  d.add_string("foo int = {bar?baz}");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Following source was not found in the environment source list: bar");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}
