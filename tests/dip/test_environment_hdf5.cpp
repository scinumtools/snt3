#include "pch_tests.h"
#include "test_environment_fixture.h"

#include <filesystem>
#include <hdf5.h>
#include <snt/dip/cursor.h>

using namespace snt;

namespace {
    class H5Handle {
      public:
        using Closer = herr_t (*)(hid_t);
        H5Handle(hid_t id, Closer closer) : id_(id), closer_(closer) {}
        H5Handle(const H5Handle&) = delete;
        ~H5Handle() {
            if (id_ >= 0)
                closer_(id_);
        }
        operator hid_t() const { return id_; }

      private:
        hid_t id_;
        Closer closer_;
    };
} // namespace

TEST(Environment, Load) {
    const auto file = environment_file("load");
    dip::Environment source = parsed_environment();
    source.save(file);
    dip::Environment env;
    env.load(file);
    ASSERT_EQ(env.nodes.size(), 8);
    EXPECT_EQ(env["title"].as<std::string>(), "C++ environment");
    EXPECT_EQ(env["simulation.steps"].as<int64_t>(), 100);
    EXPECT_DOUBLE_EQ(env["simulation.timestep"].as<double>(), 0.5);
    EXPECT_TRUE(env["simulation.enabled"].as<bool>());
    EXPECT_EQ(env.get_node("simulation.restart_file")->value, nullptr);
    EXPECT_EQ((env["boundary[inlet].velocity"].as<std::array<double, 3>>()), (std::array<double, 3>{1.0, 0.0, 0.0}));
    EXPECT_EQ(env["boundary"].items().size(), 1);
    EXPECT_EQ(env["samples"].elements().size(), 2);
    EXPECT_EQ(env["samples[1].time"].as<double>(), 1.0);
    EXPECT_EQ(env.get_node("title")->tags, (val::Array::StringType{"example", "hdf5"}));
    ASSERT_EQ(env.get_node("title")->options.size(), 2);
    EXPECT_EQ(env.get_node("title")->options[1].value_raw, "Other");
    EXPECT_EQ(env.get_node("title")->metadata.description, "Environment round-trip fixture");
    std::filesystem::remove(file);
}

TEST(Environment, SourceManifestAndCursorProvenance) {
    dip::DIP parser;
    parser.add_string("value int = 1\n");
    dip::Environment source = parser.parse();

    const dip::Provenance parsed_provenance = source["value"].get_provenance();
    ASSERT_TRUE(parsed_provenance.source.has_value());
    EXPECT_EQ(parsed_provenance.source_name, parsed_provenance.source->name);
    EXPECT_EQ(parsed_provenance.source_line, 1);
    EXPECT_EQ(parsed_provenance.source_code, "value int = 1");
    EXPECT_EQ(parsed_provenance.source->hash_algorithm, "SHA-256");
    EXPECT_EQ(parsed_provenance.source->hash, "cf4c47b9b0b584c2bfe69a84ca55d3b34513d9b9a07594e9aa879e555c1ee9ef");

    const auto file = environment_file("source-manifest");
    source.save(file);
    dip::Environment loaded;
    loaded.load(file);

    const dip::Provenance loaded_provenance = loaded["value"].get_provenance();
    ASSERT_TRUE(loaded_provenance.source.has_value());
    EXPECT_EQ(loaded_provenance.source_name, parsed_provenance.source_name);
    EXPECT_EQ(loaded_provenance.source_line, parsed_provenance.source_line);
    EXPECT_EQ(loaded_provenance.source_code, parsed_provenance.source_code);
    EXPECT_EQ(loaded_provenance.source->hash, parsed_provenance.source->hash);
    EXPECT_EQ(loaded.get_source_manifest().size(), source.get_source_manifest().size());
    std::filesystem::remove(file);
}

TEST(Environment, LoadSchemaVersion1WithoutSourceManifest) {
    const auto file = environment_file("load-version-1");
    dip::Environment source = parsed_environment();
    source.save(file);
    {
        H5Handle hdf5_file(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(hdf5_file, 0);
        ASSERT_GE(H5Ldelete(hdf5_file, "/_DIPL_Sources", H5P_DEFAULT), 0);
        H5Handle version_attribute(H5Aopen(hdf5_file, "_DIPL_Schema_Version", H5P_DEFAULT), H5Aclose);
        ASSERT_GE(version_attribute, 0);
        const uint64_t version = 1;
        ASSERT_GE(H5Awrite(version_attribute, H5T_NATIVE_UINT64, &version), 0);
    }

    dip::Environment loaded;
    loaded.load(file);
    EXPECT_TRUE(loaded.get_source_manifest().empty());
    EXPECT_FALSE(loaded["title"].get_provenance().source.has_value());
    EXPECT_EQ(loaded["title"].get_provenance().source_name, source["title"].get_provenance().source_name);
    std::filesystem::remove(file);
}

TEST(Environment, QueryLoadedNodes) {
    const auto file = environment_file("query-loaded");
    const auto source = parsed_environment();
    source.save(file);
    dip::Environment env;
    env.load(file);

    const auto expected = source.request_group("?");
    const auto actual = env.request_group("?");
    ASSERT_EQ(actual.size(), expected.size());
    for (const auto& node : actual) {
        ASSERT_NE(node, nullptr);
        const auto original = source.get_node(node->path.name);
        ASSERT_NE(original, nullptr);
        EXPECT_EQ(node->to_string(), original->to_string());
        EXPECT_NE(node.get(), env.get_node(node->path.name).get());
    }
    const auto selected = env.request_group("?", dip::RequestType::Reference, {"hdf5"});
    ASSERT_EQ(selected.size(), 1);
    ASSERT_EQ(selected.front()->options.size(), 2);
    EXPECT_EQ(selected.front()->options[1].value->to_string(), "\"Other\"");
    EXPECT_EQ(selected.front()->metadata.description, "Environment round-trip fixture");
    const auto steps = env.request_group("?simulation.steps");
    ASSERT_EQ(steps.size(), 1);
    EXPECT_EQ(steps.front()->path.name, "steps");
    EXPECT_EQ(steps.front()->to_string(), "100");
    std::filesystem::remove(file);
}

TEST(Environment, Save) {
    const auto file = environment_file("save");
    parsed_environment().save(file);
    EXPECT_TRUE(std::filesystem::is_regular_file(file));
    EXPECT_GT(std::filesystem::file_size(file), 0);
    std::filesystem::remove(file);
}

TEST(Environment, SaveHdf5Content) {
    const auto file = environment_file("save-content");
    parsed_environment().save(file);
    {
        H5Handle hdf5_file(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(hdf5_file, 0);
        H5Handle format(H5Aopen(hdf5_file, "_DIPL_Format", H5P_DEFAULT), H5Aclose);
        ASSERT_GE(format, 0);
        H5Handle type(H5Aget_type(format), H5Tclose);
        std::vector<char> text(H5Tget_size(type) + 1, '\0');
        ASSERT_GE(H5Aread(format, type, text.data()), 0);
        EXPECT_STREQ(text.data(), "SciNumTools3 Environment");
        H5Handle version_attribute(H5Aopen(hdf5_file, "_DIPL_Schema_Version", H5P_DEFAULT), H5Aclose);
        uint64_t version = 0;
        ASSERT_GE(H5Aread(version_attribute, H5T_NATIVE_UINT64, &version), 0);
        EXPECT_EQ(version, 2);
        ASSERT_GT(H5Lexists(hdf5_file, "/_DIPL_Sources", H5P_DEFAULT), 0);
        H5Handle source_manifest(H5Gopen2(hdf5_file, "/_DIPL_Sources", H5P_DEFAULT), H5Gclose);
        ASSERT_GE(source_manifest, 0);
        H5Handle source_entry(H5Gopen2(source_manifest, "0", H5P_DEFAULT), H5Gclose);
        ASSERT_GE(source_entry, 0);
        EXPECT_GT(H5Aexists(source_entry, "_DIPL_Source_Name"), 0);
        EXPECT_GT(H5Aexists(source_entry, "_DIPL_Source_Hash_Algorithm"), 0);
        EXPECT_GT(H5Aexists(source_entry, "_DIPL_Source_Hash"), 0);
        ASSERT_GT(H5Lexists(hdf5_file, "/simulation", H5P_DEFAULT), 0);
        H5Handle simulation(H5Gopen2(hdf5_file, "/simulation", H5P_DEFAULT), H5Gclose);
        EXPECT_GT(H5Aexists(simulation, "_DIPL_Kind"), 0);
        EXPECT_GT(H5Aexists(simulation, "_DIPL_Path"), 0);
        H5Handle steps(H5Dopen2(hdf5_file, "/simulation/steps", H5P_DEFAULT), H5Dclose);
        H5Handle steps_space(H5Dget_space(steps), H5Sclose);
        EXPECT_EQ(H5Sget_simple_extent_ndims(steps_space), 0);
        H5Handle steps_type(H5Dget_type(steps), H5Tclose);
        EXPECT_EQ(H5Tget_class(steps_type), H5T_INTEGER);
        EXPECT_EQ(H5Tget_size(steps_type), sizeof(int32_t));
        EXPECT_GT(H5Aexists(steps, "_DIPL_Path"), 0);
        EXPECT_GT(H5Aexists(steps, "_DIPL_Value_Type"), 0);
        H5Handle boundary(H5Gopen2(hdf5_file, "/boundary", H5P_DEFAULT), H5Gclose);
        EXPECT_GT(H5Aexists(boundary, "_DIPL_Kind"), 0);
        H5Handle inlet(H5Gopen2(hdf5_file, "/boundary/inlet", H5P_DEFAULT), H5Gclose);
        EXPECT_GT(H5Aexists(inlet, "_DIPL_Key"), 0);
        H5Handle velocity(H5Dopen2(hdf5_file, "/boundary/inlet/velocity", H5P_DEFAULT), H5Dclose);
        ASSERT_GE(velocity, 0);
        H5Handle space(H5Dget_space(velocity), H5Sclose);
        hsize_t size = 0;
        EXPECT_EQ(H5Sget_simple_extent_ndims(space), 1);
        EXPECT_EQ(H5Sget_simple_extent_dims(space, &size, nullptr), 1);
        EXPECT_EQ(size, 3);
        H5Handle velocity_type(H5Dget_type(velocity), H5Tclose);
        EXPECT_EQ(H5Tget_class(velocity_type), H5T_FLOAT);
        EXPECT_EQ(H5Tget_size(velocity_type), sizeof(double));
        EXPECT_GT(H5Aexists(velocity, "_DIPL_Array"), 0);
        EXPECT_GT(H5Aexists(velocity, "units"), 0);
        H5Handle samples(H5Gopen2(hdf5_file, "/samples", H5P_DEFAULT), H5Gclose);
        EXPECT_GT(H5Aexists(samples, "_DIPL_Kind"), 0);
        H5Handle sample_zero(H5Gopen2(hdf5_file, "/samples/0", H5P_DEFAULT), H5Gclose);
        H5Handle sample_one(H5Gopen2(hdf5_file, "/samples/1", H5P_DEFAULT), H5Gclose);
        EXPECT_GT(H5Aexists(sample_zero, "_DIPL_Kind"), 0);
        EXPECT_GT(H5Aexists(sample_one, "_DIPL_Kind"), 0);
        H5Handle title(H5Dopen2(hdf5_file, "/title", H5P_DEFAULT), H5Dclose);
        EXPECT_GT(H5Aexists(title, "description"), 0);
        EXPECT_GT(H5Aexists(title, "_DIPL_Tags"), 0);
        EXPECT_GT(H5Aexists(title, "_DIPL_Options"), 0);
    }
    std::filesystem::remove(file);
}
