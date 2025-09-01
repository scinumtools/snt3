#include "dip/dip.h"
#include "dip/environment.h"
#include "dip/nodes/nodes.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace snt;

TEST(ReferencesRaw, BooleanValues) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_raw_file.txt";
  std::string source_code = "  [[true,false], \n[false,true]]  ";
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code;
  }

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("$source " + source_name + " = " + source_filename.string());
  d.add_string("snap bool[2,2] = {" + source_name + "}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "snap");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[[true, false], [false, true]]");
}

TEST(ReferencesRaw, IntegerValues) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_raw_file.txt";
  std::string source_code = "  [[1,2], \n[3,4]]  ";
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code;
  }

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("$source " + source_name + " = " + source_filename.string());
  d.add_string("snap int[2,2] = {" + source_name + "}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "snap");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[[1, 2], [3, 4]]");
}

TEST(ReferencesRaw, FloatValues) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_raw_file.txt";
  std::string source_code = "  [[1,2.], \n[3.45,4.56e7]]  ";
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code;
  }

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("$source " + source_name + " = " + source_filename.string());
  d.add_string("snap float[2,2] = {" + source_name + "}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "snap");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[[1, 2], [3.45, 4.56e7]]");
}

TEST(ReferencesRaw, StringValues) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_raw_file.txt";
  std::string source_code = "  [[jerk,'snap'], \n[\"crackle\",'pop']]  ";
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code;
  }

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("$source " + source_name + " = " + source_filename.string());
  d.add_string("snap str[2,2] = {" + source_name + "}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "snap");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[['jerk', 'snap'], ['crackle', 'pop']]");
}

TEST(ReferencesRaw, TableNodes) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_raw_file.txt";
  std::ostringstream source_code;
  source_code << "bar int" << '\n';
  source_code << "baz bool" << '\n';
  source_code << "---" << '\n';
  source_code << "1 true" << '\n';
  source_code << "2 false" << '\n';
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code.str();
  }

  // referencing scalar and array values
  dip::DIP d;
  d.add_string("$source " + source_name + " = " + source_filename.string());
  d.add_string("snap table = {" + source_name + "}");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "snap.bar");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[1, 2]");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "snap.baz");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}
