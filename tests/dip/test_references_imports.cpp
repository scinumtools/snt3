#include "pch_tests.h"

#include <chrono>
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

TEST(ReferencesImports, ImportRemoteSchemaCollectionItems) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto source_file = std::filesystem::temp_directory_path() / ("snt-import-schema-" + suffix + ".dip");
    {
        std::ofstream source(source_file);
        ASSERT_TRUE(source.is_open());
        source << "$schema detail\n"
                  "  label str = \"ready\"\n"
                  "$schema item\n"
                  "  value int\n"
                  "  detail : detail\n"
                  "$schema unused\n"
                  "  other int = 1\n"
                  "$schema entry\n"
                  "  count int = 7\n"
                  "items map : item\n"
                  "items[key]\n"
                  "  value = 42\n"
                  "entries list : entry\n"
                  "entries[]\n";
    }

    dip::DIP full;
    full.add_source("remote", source_file.string());
    full.add_string("copy {remote?}");
    const auto full_env = full.parse();
    EXPECT_EQ(full_env["copy.items[key].value"].as<int>(), 42);
    EXPECT_EQ(full_env.hierarchy.get_collection("copy.items").items, (std::vector<std::string>{"key"}));
    EXPECT_EQ(full_env["copy.entries[0].count"].as<int>(), 7);
    EXPECT_EQ(full_env.hierarchy.get_collection("copy.entries").kind, dip::Path::Kind::List);

    dip::DIP extended;
    extended.add_source("remote", source_file.string());
    extended.add_string("copy {remote?}\ncopy.items[added]\n  value = 9\n");
    const auto extended_env = extended.parse();
    EXPECT_EQ(extended_env["copy.items[added].value"].as<int>(), 9);
    EXPECT_EQ(extended_env["copy.items[added].detail.label"].as<std::string>(), "ready");
    EXPECT_EQ(extended_env.hierarchy.get_collection("copy.items").items, (std::vector<std::string>{"key", "added"}));
    ASSERT_EQ(extended_env.get_applied_schemas("copy.items[added].value").size(), 1);
    EXPECT_EQ(extended_env.get_applied_schemas("copy.items[added].value").front().name, "item");

    dip::DIP repeated;
    repeated.add_source("remote", source_file.string());
    repeated.add_string("one {remote?items.}\ntwo {remote?items.}\n");
    const auto repeated_env = repeated.parse();
    EXPECT_EQ(repeated_env["one[key].value"].as<int>(), 42);
    EXPECT_EQ(repeated_env["two[key].value"].as<int>(), 42);

    dip::DIP unused_collision;
    unused_collision.add_source("remote", source_file.string());
    unused_collision.add_string("$schema unused\n  other int = 2\ncopy {remote?items.}\n");
    EXPECT_EQ(unused_collision.parse()["copy[key].value"].as<int>(), 42);

    dip::DIP conflict;
    conflict.add_source("remote", source_file.string());
    conflict.add_string("$schema item\n  value int = 0\ncopy {remote?items.}\n");
    try {
        (void)conflict.parse();
        FAIL() << "Import should reject a conflicting schema name";
    } catch (const dip::EnvironmentException& error) {
        EXPECT_NE(std::string(error.what()).find("Conflicting imported schema"), std::string::npos);
    }

    dip::DIP subtree;
    subtree.add_source("remote", source_file.string());
    subtree.add_string("copy {remote?items.}");
    const auto subtree_env = subtree.parse();
    EXPECT_EQ(subtree_env["copy[key].value"].as<int>(), 42);
    EXPECT_EQ(subtree_env.hierarchy.get_collection("copy").items, (std::vector<std::string>{"key"}));

    dip::DIP list_subtree;
    list_subtree.add_source("remote", source_file.string());
    list_subtree.add_string("copy {remote?entries.}");
    const auto list_env = list_subtree.parse();
    EXPECT_EQ(list_env["copy[0].count"].as<int>(), 7);
    EXPECT_EQ(list_env.hierarchy.get_collection("copy").kind, dip::Path::Kind::List);

    dip::DIP list_extended;
    list_extended.add_source("remote", source_file.string());
    list_extended.add_string("copy {remote?entries.}\ncopy[]\n");
    const auto list_extended_env = list_extended.parse();
    EXPECT_EQ(list_extended_env["copy[1].count"].as<int>(), 7);

    dip::DIP nested;
    nested.add_source("remote", source_file.string());
    nested.add_string("wrapper\n  copy {remote?items.}\n");
    const auto nested_env = nested.parse();
    EXPECT_EQ(nested_env["wrapper.copy[key].value"].as<int>(), 42);
    EXPECT_EQ(nested_env.hierarchy.get_collection("wrapper.copy").items, (std::vector<std::string>{"key"}));

    std::filesystem::remove(source_file);
}

TEST(ReferencesImports, ImportLocalKeyedCollection) {
    dip::DIP d;
    d.add_string(
        "$schema item\n"
        "  value int\n"
        "items map : item\n"
        "items[key]\n"
        "  value = 42\n"
        "copy {?items.}\n"
    );
    const auto env = d.parse();
    EXPECT_EQ(env["copy[key].value"].as<int>(), 42);
    EXPECT_EQ(env.hierarchy.get_collection("copy").items, (std::vector<std::string>{"key"}));
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
