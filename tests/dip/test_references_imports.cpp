#include "pch_tests.h"

#include <filesystem>
#include <fstream>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/inspect/inspector.h>

using namespace snt;

TEST(ReferencesImports, ImportNode) {

    // import a single node
    dip::DIP d;
    d.add_string("foo");
    d.add_string("  snap str = \"snap\"");
    d.add_string("  crackle bool[2] = [true,false]");
    d.add_string("bar {?foo.snap}");
    d.add_string("baz");
    d.add_string("  {?foo.crackle}");
    dip::Environment env = d.parse(true);
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "bar.snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"snap\"");
    const auto imported_reads = dip::Inspector{env}.graph().dependencies("?bar.snap");
    ASSERT_EQ(imported_reads.size(), 1);
    EXPECT_EQ(imported_reads.front().target, "?foo.snap");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "baz.crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(ReferencesImports, ImportNodeRemote) {

    // create temporary file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path source_filename = temp_dir / "example_source.dip";
    std::string source_code = "foo\n"
                              "  bar int = 3\n";
    std::string source_name = "foo_source";
    {
        std::ofstream source_file(source_filename);
        ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
        source_file << source_code;
    }

    // referencing scalar and array nodes
    dip::DIP d;
    d.add_string("$source " + source_name + " = \"" + source_filename.string() + "\"");
    d.add_string("pop {" + source_name + "?foo.bar}");
    dip::Environment env = d.parse(true);
    EXPECT_EQ(env.nodes.size(), 1);

    // remove temporary file
    std::filesystem::remove(source_filename);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "pop.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");
    const auto imported_reads = dip::Inspector{env}.graph().dependencies("?pop.bar");
    ASSERT_EQ(imported_reads.size(), 1);
    EXPECT_EQ(imported_reads.front().target, "foo_source?foo.bar");
}

TEST(ReferencesImports, ImportRoot) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string("foo");
    d.add_string("  snap str = \"snap\"");
    d.add_string("  crackle bool[2] = [true,false]");
    d.add_string("bar {?.}");
    d.add_string("baz");
    d.add_string("  {?foo.}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);

    dip::ValueNode::PointerType vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "bar.foo.snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"snap\"");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "bar.foo.crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[true, false]");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->path.name, "baz.snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"snap\"");

    vnode = env.nodes.at(5);
    EXPECT_EQ(vnode->path.name, "baz.crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(ReferencesImports, ImportRootRemote) {

    // create temporary file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path source_filename = temp_dir / "example_source.dip";
    std::string source_code = "foo\n"
                              "  bar int = 3\n"
                              "  baz bool[2] = [false,true]";
    std::string source_name = "foo_source";
    {
        std::ofstream source_file(source_filename);
        ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
        source_file << source_code;
    }

    // referencing scalar and array nodes
    dip::DIP d;
    d.add_string("$source " + source_name + " = \"" + source_filename.string() + "\"");
    d.add_string("pop {" + source_name + "?foo.}");
    d.add_string("jerk {" + source_name + "?.}");
    d.add_string("snap {" + source_name + "?}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);

    // remove temporary file
    std::filesystem::remove(source_filename);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "pop.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "pop.baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "jerk.foo.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "jerk.foo.baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->path.name, "snap.foo.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(5);
    EXPECT_EQ(vnode->path.name, "snap.foo.baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");
}

TEST(ReferencesLinks, LocalLinkTraversesCanonicalNodeWithoutCopyingIt) {
    dip::DIP d;
    d.add_string("ontology");
    d.add_string("  concepts map");
    d.add_string("  concepts[body]");
    d.add_string("    name str = \"body\"");
    d.add_string("    mass float = 2 kg");
    d.add_string("featured map");
    d.add_string("featured[body] link = {?ontology.concepts[body]}");

    const dip::Environment env = d.parse();
    ASSERT_EQ(env.nodes.size(), 3);
    const auto link = env.get_node("featured[body]");
    ASSERT_EQ(link->dtype, dip::NodeDtype::Link);
    EXPECT_EQ(link->link_source, "");
    EXPECT_EQ(link->link_target, "ontology.concepts[body]");
    EXPECT_EQ(env["featured"].items().size(), 1);
    EXPECT_EQ(env["featured[body].name"].get_value()->to_string(), "\"body\"");
    EXPECT_EQ(env["featured[body].mass"].get_value()->to_string(), "2");
    EXPECT_EQ(env["featured[body].mass"].get_units()->to_string(), "kg");
    const auto link_description = dip::Inspector{env}.describe("featured[body]");
    EXPECT_EQ(link_description.kind, "link");
    EXPECT_EQ(link_description.canonical_path.value(), "ontology.concepts[body]");
    EXPECT_EQ(link_description.link_status, "resolved");
    const auto value_description = dip::Inspector{env}.describe("featured[body].name");
    EXPECT_EQ(value_description.path, "featured[body].name");
    EXPECT_EQ(value_description.canonical_path.value(), "ontology.concepts[body].name");
    EXPECT_EQ(env.hierarchy.get_collection("ontology.concepts[body].name").path, "ontology.concepts[body].name");
    EXPECT_EQ(env.request_value("?featured[body]"), nullptr);

    const auto snapshot_file = std::filesystem::temp_directory_path() / "snt_local_link.diph5";
    env.save(snapshot_file);
    dip::Environment loaded;
    ASSERT_NO_THROW(loaded.load(snapshot_file));
    EXPECT_EQ(loaded["featured[body].name"].get_value()->to_string(), "\"body\"");
    std::filesystem::remove(snapshot_file);
}

TEST(ReferencesLinks, MissingTargetsAndCyclesAreRejected) {
    {
        dip::DIP d;
        d.add_string("alias link = {?missing}");
        EXPECT_THROW(d.parse(), dip::EnvironmentException);
    }
    {
        dip::DIP d;
        d.add_string("first link = {?second}");
        d.add_string("second link = {?first}");
        EXPECT_THROW(d.parse(), dip::EnvironmentException);
    }
    {
        dip::DIP d;
        d.add_string("target str = \"value\"");
        d.add_string("alias link = {?target}");
        d.add_string("alias = \"replacement\"");
        EXPECT_THROW(d.parse(), dip::SyntaxException);
    }
    {
        dip::DIP d;
        d.add_string("target map");
        d.add_string("target[item]");
        d.add_string("  value int = 1");
        d.add_string("alias link = {?target[item]}");
        d.add_string("alias.value = 2");
        try {
            (void)d.parse();
            FAIL() << "a modification through a link must be rejected";
        } catch (const dip::SyntaxException& error) {
            EXPECT_EQ(error.info().message, "Read-only link");
            EXPECT_NE(error.info().details.find("target[item].value"), std::string::npos);
        }
    }
    {
        dip::DIP d;
        d.add_string("target map");
        d.add_string("target[item]");
        d.add_string("  value int = 1");
        d.add_string("alias link = {?target[item]}");
        d.add_string("$override\n  alias.value = 2");
        try {
            (void)d.parse();
            FAIL() << "an override through a link must be rejected";
        } catch (const dip::SyntaxException& error) {
            EXPECT_EQ(error.info().message, "Read-only link");
            EXPECT_NE(error.info().details.find("target[item].value"), std::string::npos);
        }
    }
}

TEST(ReferencesLinks, RemoteLinkResolvesLiveAndSurvivesDiph5Unresolved) {
    const auto temp_dir = std::filesystem::temp_directory_path();
    const auto source_file = temp_dir / "snt_link_target.dip";
    const auto snapshot_file = temp_dir / "snt_remote_link.diph5";
    {
        std::ofstream out(source_file);
        ASSERT_TRUE(out.is_open());
        out << "catalog\n  item\n    label str = \"remote item\"\n";
    }
    dip::DIP d;
    d.add_string("$source remote = \"" + source_file.string() + "\"");
    d.add_string("alias link = {remote?catalog.item}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env["alias.label"].get_value()->to_string(), "\"remote item\"");
    env.save(snapshot_file);

    dip::Environment loaded;
    ASSERT_NO_THROW(loaded.load(snapshot_file));
    EXPECT_THROW(loaded["alias"].children(), dip::EnvironmentException);
    EXPECT_EQ(loaded.get_node("alias")->link_source, "remote");
    const auto unresolved = dip::Inspector{loaded}.describe("alias");
    EXPECT_EQ(unresolved.kind, "link");
    EXPECT_EQ(unresolved.link_status, "unresolved");
    std::filesystem::remove(source_file);
    std::filesystem::remove(snapshot_file);
}
