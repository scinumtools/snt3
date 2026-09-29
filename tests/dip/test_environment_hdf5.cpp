#include "pch_tests.h"
#include "test_environment_fixture.h"

#include <chrono>
#include <filesystem>
#include <limits>
#include <hdf5.h>
#include <snt/dip/cursor.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/inspection.h>

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

TEST(Environment, DerivedTableValuesSurviveHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "measurements table = \"\"\"speed float m/s\ntime float s\n---\n1 1\n2 2\n3 3\n\"\"\"\n"
        "distance float[3] = ({?measurements.speed} * {?measurements.time}) m\n"
    );
    const auto original = parser.parse();
    const std::vector<double> expected{1.0, 4.0, 9.0};
    EXPECT_EQ(original["distance"].as<std::vector<double>>(), expected);

    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto file = std::filesystem::temp_directory_path() / ("snt-derived-table-" + suffix + ".diph5");
    original.save(file);
    dip::Environment loaded;
    loaded.load(file);

    EXPECT_EQ(loaded["distance"].as<std::vector<double>>(), expected);
    EXPECT_EQ(loaded["distance"].get_shape(), (val::Array::ShapeType{3}));
    ASSERT_TRUE(loaded["distance"].get_units().has_value());
    EXPECT_EQ(loaded["distance"].get_units()->to_string(), "m");
    EXPECT_EQ(loaded["measurements.speed"].as<std::vector<double>>(),
              (std::vector<double>{1.0, 2.0, 3.0}));
    EXPECT_EQ(loaded["measurements.time"].as<std::vector<double>>(),
              (std::vector<double>{1.0, 2.0, 3.0}));
    std::filesystem::remove(file);
}

TEST(Environment, ExactNumericValuesSurviveHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "large_count uint64 = 9007199254740993\n"
        "maximum_count uint64 = 18446744073709551615\n"
        "minimum_offset int64 = -9223372036854775808\n"
        "gain float32 = 1.23456789\n"
        "matrix float64[2,2] = [[1.25, -2.5], [3.75, 4.5]]\n"
    );
    const auto original = parser.parse();
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto file = std::filesystem::temp_directory_path() / ("snt-numeric-fidelity-" + suffix + ".diph5");
    original.save(file);

    dip::Environment loaded;
    loaded.load(file);
    EXPECT_EQ(loaded["large_count"].as<uint64_t>(), uint64_t{9007199254740993ULL});
    EXPECT_EQ(loaded["maximum_count"].as<uint64_t>(), std::numeric_limits<uint64_t>::max());
    EXPECT_EQ(loaded["minimum_offset"].as<int64_t>(), std::numeric_limits<int64_t>::min());
    EXPECT_EQ(loaded["gain"].get_node()->value->get_dtype(), core::DataType::Float32);
    EXPECT_DOUBLE_EQ(loaded["gain"].as<double>(), static_cast<double>(static_cast<float>(1.23456789)));
    EXPECT_EQ(loaded["matrix"].get_shape(), (val::Array::ShapeType{2, 2}));
    EXPECT_EQ(loaded["matrix"].as<std::vector<double>>(),
              (std::vector<double>{1.25, -2.5, 3.75, 4.5}));
    std::filesystem::remove(file);
}

TEST(Environment, RejectsCorruptModificationHistoryWithoutReplacingEnvironment) {
    dip::DIP parser;
    parser.add_string("answer int = 1\nanswer = 2\n");
    const auto source = parser.parse();
    auto env = source;
    const auto file = environment_file("corrupt-modification-history");

    auto corrupt_history = [&](bool remove_lines) {
        source.save(file);
        H5Handle h5file(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(static_cast<hid_t>(h5file), 0);
        H5Handle dataset(H5Dopen2(h5file, "/answer", H5P_DEFAULT), H5Dclose);
        ASSERT_GE(static_cast<hid_t>(dataset), 0);
        constexpr const char* lines_attribute = "_DIPL_Modification_Lines";
        if (remove_lines) {
            ASSERT_GE(H5Adelete(dataset, lines_attribute), 0);
        } else {
            H5Handle attribute(H5Aopen(dataset, lines_attribute, H5P_DEFAULT), H5Aclose);
            ASSERT_GE(static_cast<hid_t>(attribute), 0);
            H5Handle type(H5Aget_type(attribute), H5Tclose);
            ASSERT_GE(static_cast<hid_t>(type), 0);
            const char invalid_line[] = "x";
            ASSERT_GE(H5Awrite(attribute, type, invalid_line), 0);
        }
    };

    for (const bool remove_lines : {true, false}) {
        SCOPED_TRACE(remove_lines ? "missing lines" : "invalid line number");
        corrupt_history(remove_lines);
        try {
            dip::reload_artifact(env, file);
            FAIL() << "Expected corrupted modification history to be rejected";
        } catch (const dip::IOException& error) {
            EXPECT_EQ(error.info().message, "Invalid value modification history");
        }
        EXPECT_EQ(env["answer"].as<int64_t>(), 2);
        EXPECT_EQ(env.nodes.size(), 1);
    }
    std::filesystem::remove(file);
}

TEST(Environment, ExtendedMetadataHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "theta float = 0.7\n"
        "  ?rationale \"Balances accuracy and cost\"\n"
        "  ?native [\"ErrTolTheta\", \"TreeOpeningAngle\"]\n"
        "  ?requires [\"build.gravity.self_gravity\", \"build.gravity.tree\"]\n"
        "  ?conflicts \"build.gravity.direct\"\n"
        "  ?implies \"build.gravity.tree\"\n"
        "  ?see [\"gravity.force_accuracy\", \"gravity.softening\"]\n"
        "  ?example [\"0.5\", \"0.7\"]\n"
        "  ?recommended_range \"0.3 to 1.0\"\n"
        "  ?performance_impact \"Smaller values cost more\"\n"
        "  ?scientific_impact \"Smaller values improve accuracy\"\n"
        "  ?deprecated \"Never\"\n"
        "  ?replacement \"gravity.opening_angle\"\n"
        "  ?since \"3.0\"\n"
        "  ?category \"Gravity\"\n"
        "  ?visibility \"advanced\"\n"
    );
    const auto file = environment_file("extended-metadata");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    const auto& metadata = loaded.get_node("theta")->metadata;
    EXPECT_EQ(metadata.rationale, "Balances accuracy and cost");
    EXPECT_EQ(metadata.native, (std::vector<std::string>{"ErrTolTheta", "TreeOpeningAngle"}));
    EXPECT_EQ(metadata.requires, (std::vector<std::string>{"build.gravity.self_gravity", "build.gravity.tree"}));
    EXPECT_EQ(metadata.conflicts, (std::vector<std::string>{"build.gravity.direct"}));
    EXPECT_EQ(metadata.implies, (std::vector<std::string>{"build.gravity.tree"}));
    EXPECT_EQ(metadata.see, (std::vector<std::string>{"gravity.force_accuracy", "gravity.softening"}));
    EXPECT_EQ(metadata.example, (std::vector<std::string>{"0.5", "0.7"}));
    EXPECT_EQ(metadata.recommended_range, "0.3 to 1.0");
    EXPECT_EQ(metadata.performance_impact, "Smaller values cost more");
    EXPECT_EQ(metadata.scientific_impact, "Smaller values improve accuracy");
    EXPECT_EQ(metadata.deprecated, "Never");
    EXPECT_EQ(metadata.replacement, "gravity.opening_angle");
    EXPECT_EQ(metadata.since, "3.0");
    EXPECT_EQ(metadata.category, "Gravity");
    EXPECT_EQ(metadata.visibility, "advanced");
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

TEST(Environment, TraceManifestRoundTrip) {
    dip::DIP parser;
    parser.add_unit("trace_round_trip_unit", "m");
    parser.add_function_value("trace_round_trip_function", [](const dip::Environment&) -> dip::ValueNodeData {
        return {};
    });
    parser.add_string(
        "$schema trace_round_trip_schema\n"
        "  value int\n"
        "record : trace_round_trip_schema\n"
        "  value = 1\n"
    );
    const dip::Environment source = parser.parse();
    const std::vector<dip::TraceInfo> expected = source.get_trace_manifest();
    ASSERT_EQ(expected.size(), 3);

    const auto file = environment_file("trace-manifest");
    source.save(file);
    dip::Environment loaded;
    loaded.load(file);
    const std::vector<dip::TraceInfo> actual = loaded.get_trace_manifest();
    ASSERT_EQ(actual.size(), expected.size());
    for (size_t index = 0; index < expected.size(); ++index) {
        EXPECT_EQ(actual[index].id, expected[index].id);
        EXPECT_EQ(actual[index].name, expected[index].name);
        EXPECT_EQ(actual[index].kind, expected[index].kind);
    }
    std::filesystem::remove(file);
}

TEST(Environment, SchemaProvenanceHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_schema_string(
        "settings", "?descr \"Reusable settings\"\n?doi \"10.1234/settings\"\nvalue int = 42\n"
    );
    parser.add_string("physics : settings\nitems list : settings\nitems[]\n");
    const dip::Environment source = parser.parse();
    const auto before = source.get_schema_manifest();
    ASSERT_EQ(before.size(), 1);
    ASSERT_TRUE(before.front().source.has_value());
    ASSERT_EQ(source.get_applied_schemas("physics.value").size(), 1);
    ASSERT_EQ(source.get_applied_schemas("items[0].value").size(), 1);
    ASSERT_TRUE(source.get_contributing_schema("physics.value").has_value());

    const auto file = environment_file("schema-provenance");
    source.save(file);
    dip::Environment loaded;
    loaded.load(file);
    EXPECT_TRUE(loaded.schemas.entries().empty());
    const auto after = loaded.get_schema_manifest();
    ASSERT_EQ(after.size(), 1);
    EXPECT_EQ(after.front().id, before.front().id);
    EXPECT_EQ(after.front().name, "settings");
    EXPECT_EQ(after.front().metadata.description, "Reusable settings");
    EXPECT_EQ(after.front().metadata.doi, "10.1234/settings");
    EXPECT_EQ(after.front().source_name, before.front().source_name);
    EXPECT_EQ(after.front().source_line, before.front().source_line);
    ASSERT_TRUE(after.front().source.has_value());
    EXPECT_EQ(after.front().source->hash, before.front().source->hash);
    ASSERT_EQ(loaded.get_applied_schemas("physics.value").size(), 1);
    EXPECT_EQ(loaded.get_applied_schemas("physics.value").front().metadata.description, "Reusable settings");
    ASSERT_EQ(loaded.get_applied_schemas("items[0].value").size(), 1);
    ASSERT_TRUE(loaded.get_contributing_schema("physics.value").has_value());
    EXPECT_EQ(loaded.get_contributing_schema("physics.value")->id, before.front().id);
    EXPECT_TRUE(loaded.get_applied_schemas("items").empty());
    EXPECT_EQ(loaded["physics.value"].as<int64_t>(), 42);

    const auto copy = environment_file("schema-provenance-copy");
    loaded.save(copy);
    dip::Environment reloaded;
    reloaded.load(copy);
    ASSERT_EQ(reloaded.get_schema_manifest().size(), 1);
    EXPECT_EQ(reloaded.get_schema_manifest().front().metadata.doi, "10.1234/settings");
    std::filesystem::remove(copy);
    std::filesystem::remove(file);
}

TEST(Environment, InlineSchemaMetadataAndOverrideHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "$schema inline_settings\n"
        "  ?descr \"Inline description\"\n"
        "  ?doi \"10.1234/inline\"\n"
        "  value int = 1\n"
        "record : inline_settings\n"
        "  value = 2\n"
        "plain int = 3\n"
    );
    const auto file = environment_file("inline-schema-provenance");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    EXPECT_TRUE(loaded.schemas.entries().empty());
    ASSERT_EQ(loaded.get_schema_manifest().size(), 1);
    EXPECT_EQ(loaded.get_schema_manifest().front().metadata.description, "Inline description");
    EXPECT_EQ(loaded.get_schema_manifest().front().metadata.doi, "10.1234/inline");
    EXPECT_EQ(loaded.get_schema_manifest().front().source_line, 1);
    ASSERT_TRUE(loaded.get_contributing_schema("record.value").has_value());
    EXPECT_EQ(loaded.get_contributing_schema("record.value")->name, "inline_settings");
    EXPECT_FALSE(loaded.get_contributing_schema("plain").has_value());
    EXPECT_EQ(loaded["record.value"].as<int64_t>(), 2);
    std::filesystem::remove(file);
}

TEST(Environment, OverrideProvenanceHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string("$override\n  radius = 20 cm\nradius float = 10 cm\n");
    const auto source = parser.parse();
    const auto file = environment_file("override-provenance");
    source.save(file);
    dip::Environment loaded;
    loaded.load(file);
    EXPECT_TRUE(loaded.get_node("radius")->override);
    EXPECT_EQ(loaded["radius"].as<double>(), 20);
    const auto provenance = loaded["radius"].get_provenance();
    EXPECT_EQ(provenance.source_line, 3);
    EXPECT_EQ(provenance.override_line, 2);
    EXPECT_EQ(provenance.override_code, "  radius = 20 cm");
    ASSERT_TRUE(provenance.override_source.has_value());
    EXPECT_EQ(provenance.override_source->name, provenance.source_name);
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
        H5Handle minor_version_attribute(H5Aopen(hdf5_file, "_DIPL_Schema_Version_Minor", H5P_DEFAULT), H5Aclose);
        uint64_t minor_version = 0;
        ASSERT_GE(H5Aread(minor_version_attribute, H5T_NATIVE_UINT64, &minor_version), 0);
        EXPECT_EQ(minor_version, 6);
        ASSERT_GT(H5Lexists(hdf5_file, "/_DIPL_Sources", H5P_DEFAULT), 0);
        ASSERT_GT(H5Lexists(hdf5_file, "/_DIPL_Trace", H5P_DEFAULT), 0);
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

TEST(Environment, ValueNodeWithChildrenHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "feature bool = true\n"
        "  setting int = 2\n"
        "  cutoff float = 5 m\n"
    );
    const dip::Environment source = parser.parse();
    const auto file = environment_file("value-node-children");
    source.save(file);

    {
        H5Handle hdf5_file(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(hdf5_file, 0);
        H5Handle feature(H5Gopen2(hdf5_file, "/feature", H5P_DEFAULT), H5Gclose);
        ASSERT_GE(feature, 0);
        EXPECT_GT(H5Aexists(feature, "_DIPL_Kind"), 0);
        H5Handle payload(H5Dopen2(feature, "_DIPL_Value", H5P_DEFAULT), H5Dclose);
        ASSERT_GE(payload, 0);
        EXPECT_GT(H5Aexists(payload, "_DIPL_Value_Type"), 0);
        EXPECT_GT(H5Lexists(feature, "setting", H5P_DEFAULT), 0);
    }

    dip::Environment loaded;
    loaded.load(file);
    EXPECT_TRUE(loaded["feature"].as<bool>());
    EXPECT_EQ(loaded["feature.setting"].as<int64_t>(), 2);
    EXPECT_DOUBLE_EQ(loaded["feature.cutoff"].as<double>(), 5.0);
    EXPECT_EQ(loaded.get_node("feature.cutoff")->units->to_string(), "m");

    const auto resaved = environment_file("value-node-children-resaved");
    loaded.save(resaved);
    dip::Environment round_tripped;
    round_tripped.load(resaved);
    EXPECT_TRUE(round_tripped["feature"].as<bool>());
    EXPECT_EQ(round_tripped["feature.setting"].as<int64_t>(), 2);
    std::filesystem::remove(file);
    std::filesystem::remove(resaved);
}

TEST(Environment, RejectsReservedValueGroupPayloadChild) {
    dip::DIP parser;
    parser.add_string(
        "feature bool = true\n"
        "  _DIPL_Value int = 2\n"
    );
    const auto file = environment_file("reserved-value-payload");
    EXPECT_THROW(parser.parse().save(file), dip::IOException);
    std::filesystem::remove(file);
}

TEST(Environment, CustomUnitsHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "$unit arepo_length = 2*m\n"
        "$unit arepo_span = 3*arepo_length\n"
        "box_size float = 7500 arepo_span\n"
    );
    const dip::Environment source = parser.parse();
    const auto file = environment_file("custom-units");
    source.save(file);

    dip::Environment loaded;
    loaded.load(file);
    EXPECT_DOUBLE_EQ(loaded["box_size"].as<double>(), 7500.0);
    EXPECT_EQ(loaded.get_node("box_size")->units->to_string(), "arepo_span");
    const dip::EnvUnit& length = loaded.units.at("arepo_length");
    const dip::EnvUnit& span = loaded.units.at("arepo_span");
    EXPECT_EQ(length.definition, "2*m");
    EXPECT_EQ(span.definition, "3*arepo_length");
    EXPECT_LT(length.registration_order, span.registration_order);

    const auto resaved = environment_file("custom-units-resaved");
    loaded.save(resaved);
    dip::Environment round_tripped;
    round_tripped.load(resaved);
    EXPECT_EQ(round_tripped.units.at("arepo_span").id, span.id);
    EXPECT_EQ(round_tripped.units.at("arepo_span").definition, span.definition);
    std::filesystem::remove(file);
    std::filesystem::remove(resaved);
}

TEST(Environment, IndexedListItemModificationHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "softenings[]\n"
        "  length float = 1 cm\n"
        "softenings[]\n"
        "  length float = 2 cm\n"
        "softenings[1]\n"
        "  length = 3 cm\n"
    );
    const dip::Environment source = parser.parse();
    const auto file = environment_file("indexed-list-modification");
    source.save(file);
    dip::Environment loaded;
    loaded.load(file);
    EXPECT_DOUBLE_EQ(loaded["softenings[1].length"].as<double>(), 3.0);
    EXPECT_EQ(loaded.hierarchy.get_collection("softenings").items, (std::vector<std::string>{"0", "1"}));
    std::filesystem::remove(file);
}
