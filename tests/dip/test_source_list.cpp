#include "pch_tests.h"
#include <snt/dip/dip.h>

#include <filesystem>
#include <fstream>

using namespace snt;

TEST(SourceList, KeywordSourceCode) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_source.dip";
  std::string source_code = "foo int = 3\nbar bool = false";
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code;
  }

  // load a DIP file as a source
  dip::DIP d;
  d.add_string("$source " + source_name + " = " + source_filename.string());
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 0);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::EnvSource& senv = env.sources.at(source_name);
  EXPECT_EQ(senv.name, source_name);
  EXPECT_EQ(senv.path, source_filename);
  EXPECT_EQ(senv.code, source_code);
  EXPECT_FALSE(senv.parent.name.empty());
  EXPECT_EQ(senv.nodes.size(), 2);

  dip::ValueNode::PointerType vnode = senv.nodes.at(0);
  EXPECT_EQ(vnode->name, "foo");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "3");

  vnode = senv.nodes.at(1);
  EXPECT_EQ(vnode->name, "bar");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "false");
}

TEST(SourceList, KeywordSourceText) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_text.txt";
  std::string source_code = "Hello World!";
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code;
  }

  // load a DIP file as a source
  dip::DIP d;
  d.add_string("$source " + source_name + " = " + source_filename.string());
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 0);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::EnvSource& senv = env.sources.at(source_name);
  EXPECT_EQ(senv.name, source_name);
  EXPECT_EQ(senv.path, source_filename);
  EXPECT_EQ(senv.code, source_code);
  EXPECT_FALSE(senv.parent.name.empty());
  EXPECT_EQ(senv.nodes.size(), 0);
}

TEST(SourceList, AddSourceCode) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path source_filename = temp_dir / "example_source.dip";
  std::string source_code = "foo int = 3\nbar bool = false";
  std::string source_name = "foo_source";
  {
    std::ofstream source_file(source_filename);
    ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
    source_file << source_code;
  }

  // load a DIP file as a source
  dip::DIP d;
  d.add_source(source_name, source_filename.string());
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 0);

  // remove temporary file
  std::filesystem::remove(source_filename);

  dip::EnvSource& senv = env.sources.at(source_name);
  EXPECT_EQ(senv.name, source_name);
  EXPECT_EQ(senv.path, source_filename);
  EXPECT_EQ(senv.code, source_code);
  EXPECT_FALSE(senv.parent.name.empty());
  EXPECT_EQ(senv.nodes.size(), 2);

  dip::ValueNode::PointerType vnode = senv.nodes.at(0);
  EXPECT_EQ(vnode->name, "foo");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "3");

  vnode = senv.nodes.at(1);
  EXPECT_EQ(vnode->name, "bar");
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "false");
}
