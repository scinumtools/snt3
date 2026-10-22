#include "pch_tests.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

using namespace snt;

TEST(SourceList, KeywordSourceCode) {

    // create temporary file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path source_filename = temp_dir / "example_source.dip";
    std::string source_code = "foo int = 3\nbar bool = false";
    std::string source_name = "foo_source";
    {
        std::ofstream source_file(source_filename, std::ios::binary);
        ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
        source_file << source_code;
    }

    // load a DIP file as a source
    dip::DIP d;
    d.add_string("$source " + source_name + " = \"" + source_filename.string() + "\"");
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
    EXPECT_EQ(vnode->path.name, "foo");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = senv.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
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
    d.add_string("$source " + source_name + " = \"" + source_filename.string() + "\"");
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

TEST(SourceList, RelativePathsUseDeclaringFileDirectory) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto root = std::filesystem::temp_directory_path() / ("snt-relative-source-" + suffix);
    std::filesystem::create_directories(root / "sub" / "deeper");
    {
        std::ofstream main(root / "main.dip");
        main << "$source child = \"sub/child.dip\"\n"
                "$source raw = \"sub/note.txt\"\n"
                "answer int = {child?answer}\n";
        std::ofstream child(root / "sub" / "child.dip");
        child << "$source leaf = \"deeper/leaf.dip\"\n"
                 "answer int = {leaf?value}\n";
        std::ofstream leaf(root / "sub" / "deeper" / "leaf.dip");
        leaf << "value int = 42\n";
        std::ofstream raw(root / "sub" / "note.txt", std::ios::binary);
        raw << "relative raw source\n";
    }

    dip::DIP d;
    d.add_file(root / "main.dip");
    const dip::Environment env = d.parse();
    EXPECT_EQ(env["answer"].as<int>(), 42);
    EXPECT_EQ(env.sources.at("child").path, (root / "sub" / "child.dip").string());
    EXPECT_EQ(env.sources.at("raw").path, (root / "sub" / "note.txt").string());
    EXPECT_EQ(env.sources.at("raw").code, "relative raw source\n");

    std::filesystem::remove_all(root);
}

TEST(SourceList, AddSourceFromCode) {

    // create temporary file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path source_filename = temp_dir / "example_source.dip";
    std::string source_code = "foo int = 3\nbar bool = false";
    std::string source_name = "foo_source";
    {
        std::ofstream source_file(source_filename, std::ios::binary);
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
    EXPECT_EQ(vnode->path.name, "foo");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = senv.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "false");
}

TEST(SourceList, ValueInjection) {
    const std::filesystem::path source_filename = std::filesystem::temp_directory_path() / "injected_source.dip";
    {
        std::ofstream source_file(source_filename);
        ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
        source_file << "answer int = 42\n";
    }

    dip::DIP d;
    d.add_string("source_path str = \"" + source_filename.string() + "\"");
    d.add_string("$source injected = {?source_path}");
    dip::Environment env = d.parse();

    std::filesystem::remove(source_filename);

    const dip::EnvSource& source = env.sources.at("injected");
    EXPECT_EQ(source.path, source_filename);
    ASSERT_EQ(source.nodes.size(), 1);
    EXPECT_EQ(source.nodes.at(0)->path.name, "answer");
    EXPECT_EQ(source.nodes.at(0)->value->to_string(), "42");
}

TEST(SourceList, ValueInjectionRequiresScalarString) {
    dip::DIP d;
    d.add_string("source_path int = 42");
    d.add_string("$source injected = {?source_path}");

    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Invalid source reference value");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}
