#include "pch_tests.h"
#include "test_environment_fixture.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <hdf5.h>
#include <limits>
#include <snt/dip/artifact.h>
#include <snt/dip/cursor.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/inspect/inspector.h>
#include <sstream>

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

    void remove_order_attributes(hid_t group) {
        hsize_t count = 0;
        ASSERT_GE(H5Gget_num_objs(group, &count), 0);
        for (hsize_t index = 0; index < count; ++index) {
            const ssize_t length = H5Gget_objname_by_idx(group, index, nullptr, 0);
            ASSERT_GE(length, 0);
            std::string name(static_cast<size_t>(length) + 1, '\0');
            ASSERT_GE(H5Gget_objname_by_idx(group, index, name.data(), name.size()), 0);
            name.resize(static_cast<size_t>(length));
            const int type = H5Gget_objtype_by_idx(group, index);
            ASSERT_GE(type, 0);
            H5Handle object(H5Oopen(group, name.c_str(), H5P_DEFAULT), H5Oclose);
            if (type == H5G_GROUP)
                remove_order_attributes(object);
            if (H5Aexists(object, "_DIPL_Order") > 0)
                ASSERT_GE(H5Adelete(object, "_DIPL_Order"), 0);
        }
    }
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

TEST(Environment, CollectionOrderSurvivesHdf5RoundTrip) {
    dip::DIP parser;
    std::string source =
        "zeta\n"
        "  second int = 2\n"
        "  first int = 1\n"
        "alpha\n"
        "  value int = 3\n"
        "catalog map\n"
        "catalog[zeta]\n"
        "  value int = 4\n"
        "catalog[alpha]\n"
        "  value int = 5\n"
        "numbers list\n";
    for (int index = 0; index < 12; ++index)
        source += "numbers[]\n  value int = " + std::to_string(index) + "\n";
    parser.add_string(source);
    const dip::Environment original = parser.parse();

    const auto file = environment_file("collection-order");
    original.save(file);
    dip::Environment loaded;
    loaded.load(file);

    EXPECT_EQ(loaded.hierarchy.root_children_order(), (std::vector<std::string>{"zeta", "alpha", "catalog", "numbers"}));
    EXPECT_EQ(loaded.hierarchy.get_collection("zeta").children, (std::vector<std::string>{"second", "first"}));
    EXPECT_EQ(loaded.hierarchy.get_collection("catalog").items, (std::vector<std::string>{"zeta", "alpha"}));
    std::vector<std::string> expected_indices;
    for (int index = 0; index < 12; ++index)
        expected_indices.push_back(std::to_string(index));
    EXPECT_EQ(loaded.hierarchy.get_collection("numbers").items, expected_indices);

    const auto round_trip = environment_file("collection-order-resaved");
    loaded.save(round_trip);
    dip::Environment reloaded;
    reloaded.load(round_trip);
    EXPECT_EQ(reloaded.hierarchy.root_children_order(), loaded.hierarchy.root_children_order());
    EXPECT_EQ(reloaded.hierarchy.get_collection("zeta").children, loaded.hierarchy.get_collection("zeta").children);
    EXPECT_EQ(reloaded.hierarchy.get_collection("catalog").items, loaded.hierarchy.get_collection("catalog").items);
    EXPECT_EQ(reloaded.hierarchy.get_collection("numbers").items, expected_indices);

    const auto legacy_file = environment_file("collection-order-legacy");
    std::filesystem::copy_file(file, legacy_file, std::filesystem::copy_options::overwrite_existing);
    {
        H5Handle legacy(H5Fopen(legacy_file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        remove_order_attributes(legacy);
        H5Handle minor(H5Aopen(legacy, "_DIPL_Schema_Version_Minor", H5P_DEFAULT), H5Aclose);
        const uint64_t legacy_minor = 9;
        ASSERT_GE(H5Awrite(minor, H5T_NATIVE_UINT64, &legacy_minor), 0);
    }
    dip::Environment legacy_loaded;
    legacy_loaded.load(legacy_file);
    EXPECT_EQ(legacy_loaded.hierarchy.root_children_order(), (std::vector<std::string>{"alpha", "catalog", "numbers", "zeta"}));
    EXPECT_EQ(legacy_loaded.hierarchy.get_collection("zeta").children, (std::vector<std::string>{"first", "second"}));
    EXPECT_EQ(legacy_loaded.hierarchy.get_collection("catalog").items, (std::vector<std::string>{"alpha", "zeta"}));
    EXPECT_EQ(legacy_loaded.hierarchy.get_collection("numbers").items, expected_indices);

    std::filesystem::remove(file);
    std::filesystem::remove(round_trip);
    std::filesystem::remove(legacy_file);
}

TEST(Environment, OptionLabelsSurviveHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string("level int = 2\n  !options [1, 2]\n  !labels [\"Low\", \"High\"]\n");
    const auto file = environment_file("option-labels");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    const auto node = loaded.get_node("level");
    ASSERT_EQ(node->options.size(), 2);
    EXPECT_EQ(node->options[0].label, "Low");
    EXPECT_EQ(node->options[1].label, "High");
    EXPECT_EQ(dip::Inspector{loaded}.describe("level").option_labels, (std::vector<std::string>{"Low", "High"}));
    std::filesystem::remove(file);
}

TEST(Environment, HierarchyNodeMetadataSurvivesHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "ontology\n"
        "  ?descr \"Concept register\"\n"
        "  sources map\n"
        "  sources[st_i_3_7]\n"
        "    ?authors \"Sample authors\"\n"
        "    ?title \"\"\n"
        "    locator str = \"I, q. 3, a. 7\"\n"
    );
    const auto file = environment_file("hierarchy-node-metadata");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    const dip::Inspector view{loaded};
    const auto group = view.describe("ontology");
    EXPECT_EQ(group.metadata.description, "Concept register");
    const auto source = view.describe("ontology.sources[st_i_3_7]");
    EXPECT_EQ(source.metadata.authors, "Sample authors");
    EXPECT_TRUE(source.metadata.has_property(dip::PropertyType::Title));
    EXPECT_TRUE(source.metadata.title.empty());
    std::filesystem::remove(file);
}

TEST(Environment, EmptyStringsSurviveHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string("empty str = \"\"\nvalues str[2] = [\"\", \"sample\"]\n");
    const auto file = environment_file("empty-strings");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    EXPECT_EQ(loaded["empty"].get_value()->to_string(), "\"\"");
    EXPECT_EQ(loaded["values"].get_value()->to_string(), "[\"\", \"sample\"]");
    std::filesystem::remove(file);
}

TEST(Environment, TableAndColumnMetadataSurviveHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "measurements table = \"\"\"\n"
        "?descr \"Experiment measurements\"\n"
        "time float s\n  ?descr \"Elapsed time\"\n"
        "speed float m/s\n  ?descr \"Measured speed\"\n"
        "---\n0 3\n1 4\n\"\"\"\n"
    );
    const auto file = environment_file("table-metadata");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    const dip::Inspector view{loaded};
    EXPECT_EQ(view.describe("measurements").metadata.description, "Experiment measurements");
    const auto table = view.table("measurements");
    ASSERT_EQ(table.columns.size(), 2);
    EXPECT_EQ(table.columns[0].metadata.description, "Elapsed time");
    EXPECT_EQ(table.columns[1].metadata.description, "Measured speed");
    std::filesystem::remove(file);
}

TEST(Environment, SourceMetadataSurvivesHdf5RoundTrip) {
    dip::DIP parser;
    parser.add_string(
        "?authors \"Sample research group\"\n"
        "?title \"Example source\"\n"
        "concept str = \"substance\"\n",
        "ontology.dip"
    );
    const auto file = environment_file("source-metadata");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    const auto source = loaded.get_source_info("ontology.dip");
    ASSERT_TRUE(source);
    EXPECT_EQ(source->metadata.authors, "Sample research group");
    EXPECT_EQ(source->metadata.title, "Example source");
    EXPECT_TRUE(source->metadata.has_property(dip::PropertyType::Authors));
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
    EXPECT_EQ(loaded["measurements.speed"].as<std::vector<double>>(), (std::vector<double>{1.0, 2.0, 3.0}));
    EXPECT_EQ(loaded["measurements.time"].as<std::vector<double>>(), (std::vector<double>{1.0, 2.0, 3.0}));
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
    EXPECT_EQ(loaded["matrix"].as<std::vector<double>>(), (std::vector<double>{1.25, -2.5, 3.75, 4.5}));
    std::filesystem::remove(file);
}

TEST(Environment, ArrayShapeSurvivesHdf5AndStructuredExport) {
    dip::DIP parser;
    parser.add_string(
        "scalar str = \"example\"\n"
        "singleton str[1] = [\"example\"]\n"
        "matrix int[2,2] = [[1,2],[3,4]]\n"
    );
    const auto original = parser.parse();
    const auto snapshot = environment_file("array-shape-roundtrip");
    const auto json_file = std::filesystem::path(snapshot.string() + ".json");
    original.save(snapshot);

    dip::Environment loaded;
    loaded.load(snapshot);
    ASSERT_NE(loaded.get_node("scalar"), nullptr);
    ASSERT_NE(loaded.get_node("singleton"), nullptr);
    ASSERT_NE(loaded.get_node("matrix"), nullptr);
    EXPECT_TRUE(loaded.get_node("scalar")->dimension.empty());
    ASSERT_EQ(loaded.get_node("singleton")->dimension.size(), 1);
    EXPECT_EQ(loaded.get_node("singleton")->dimension.front().dmin, 1);
    ASSERT_EQ(loaded.get_node("matrix")->dimension.size(), 2);
    EXPECT_EQ(loaded.get_node("matrix")->dimension[0].dmin, 2);
    EXPECT_EQ(loaded.get_node("matrix")->dimension[1].dmin, 2);

    loaded.generate(dip::ExportFormat::JSON_METADATA, json_file);
    std::ifstream generated(json_file);
    std::stringstream content;
    content << generated.rdbuf();
    const auto json = content.str();
    EXPECT_NE(
        json.find("\"scalar\": {\"name\": \"scalar\", \"type\": \"str\", \"shape\": [], \"is_array\": false"),
        std::string::npos
    );
    EXPECT_NE(
        json.find("\"singleton\": {\"name\": \"singleton\", \"type\": \"str\", \"shape\": [1], \"is_array\": true"),
        std::string::npos
    );
    EXPECT_NE(json.find("\"value\": \"example\""), std::string::npos);
    EXPECT_NE(
        json.find("\"matrix\": {\"name\": \"matrix\", \"type\": \"int32\", \"shape\": [2, 2], \"is_array\": true"),
        std::string::npos
    );
    generated.close();
    std::filesystem::remove(snapshot);
    std::filesystem::remove(json_file);
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

TEST(Environment, ChecksModificationLineNumberAgainstSizeT) {
    dip::DIP parser;
    parser.add_string("answer int = 1\nanswer = 2\n");
    const auto file = environment_file("large-modification-line");
    parser.parse().save(file);
    constexpr uint64_t large_line = uint64_t{1} << 32;
    {
        H5Handle h5file(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(static_cast<hid_t>(h5file), 0);
        H5Handle dataset(H5Dopen2(h5file, "/answer", H5P_DEFAULT), H5Dclose);
        ASSERT_GE(static_cast<hid_t>(dataset), 0);
        constexpr const char* name = "_DIPL_Modification_Lines";
        ASSERT_GE(H5Adelete(dataset, name), 0);
        const hsize_t count = 1;
        H5Handle space(H5Screate_simple(1, &count, nullptr), H5Sclose);
        H5Handle type(H5Tcopy(H5T_C_S1), H5Tclose);
        ASSERT_GE(H5Tset_size(type, 11), 0);
        H5Handle attribute(H5Acreate2(dataset, name, type, space, H5P_DEFAULT, H5P_DEFAULT), H5Aclose);
        ASSERT_GE(static_cast<hid_t>(attribute), 0);
        const char text[11] = "4294967296";
        ASSERT_GE(H5Awrite(attribute, type, text), 0);
    }

    dip::Environment loaded;
    if (large_line > std::numeric_limits<size_t>::max()) {
        EXPECT_THROW(loaded.load(file), dip::IOException);
    } else {
        loaded.load(file);
        ASSERT_EQ(loaded.get_node("answer")->modification_lines.size(), 1);
        EXPECT_EQ(loaded.get_node("answer")->modification_lines.front().source.line_number, large_line);
    }
    std::filesystem::remove(file);
}

TEST(Environment, ChecksDependencyEventLineNumberAgainstSizeT) {
    dip::DIP parser;
    parser.add_string("answer int = 1\n");
    const auto file = environment_file("large-dependency-line");
    parser.parse(true).save(file);
    constexpr uint64_t large_line = uint64_t{1} << 32;
    {
        H5Handle h5file(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(static_cast<hid_t>(h5file), 0);
        H5Handle event(H5Gopen2(h5file, "/_DIPL_Dependencies/0", H5P_DEFAULT), H5Gclose);
        ASSERT_GE(static_cast<hid_t>(event), 0);
        H5Handle attribute(H5Aopen(event, "line", H5P_DEFAULT), H5Aclose);
        ASSERT_GE(static_cast<hid_t>(attribute), 0);
        ASSERT_GE(H5Awrite(attribute, H5T_NATIVE_UINT64, &large_line), 0);
    }

    dip::Environment loaded;
    if (large_line > std::numeric_limits<size_t>::max()) {
        EXPECT_THROW(loaded.load(file), dip::IOException);
    } else {
        loaded.load(file);
        const dip::Inspector view{loaded};
        ASSERT_FALSE(view.graph().events.empty());
        ASSERT_TRUE(view.graph().events.front().location.has_value());
        EXPECT_EQ(view.graph().events.front().location->line, large_line);
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

TEST(Environment, RelativeSourcePathsFollowSnapshotLocation) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto root = std::filesystem::temp_directory_path() / ("snt-relative-sources-" + suffix);
    const auto source_dir = root / "sources";
    const auto snapshot_dir = root / "snapshots";
    const auto copy_dir = root / "copies";
    std::filesystem::create_directories(source_dir);
    std::filesystem::create_directories(snapshot_dir);
    std::filesystem::create_directories(copy_dir);
    const auto source_file = source_dir / "values.dip";
    {
        std::ofstream stream(source_file);
        stream << "value int = 1\n";
    }
    dip::DIP parser;
    parser.add_file(source_file.string());
    const auto parsed = parser.parse(true);
    ASSERT_TRUE(dip::Inspector{parsed}.graph().recorded);

    dip::SnapshotSaveOptions options;
    options.source_paths = dip::SourcePathPolicy::RelativeToSnapshot;
    const auto snapshot = snapshot_dir / "values.diph5";
    parsed.save(snapshot, options);
    dip::Environment loaded;
    loaded.load(snapshot);
    ASSERT_TRUE(dip::Inspector{loaded}.graph().recorded);
    const auto manifest = loaded.get_source_manifest();
    const auto source_it = std::find_if(manifest.begin(), manifest.end(), [&](const dip::SourceInfo& info) {
        return std::filesystem::path(info.path).filename() == source_file.filename();
    });
    ASSERT_NE(source_it, manifest.end());
    const auto& source = *source_it;
    EXPECT_FALSE(std::filesystem::path(source.path).is_absolute());
    EXPECT_EQ((snapshot_dir / source.path).lexically_normal(), source_file);

    const auto copy = copy_dir / "values.diph5";
    loaded.save(copy, options);
    dip::Environment reloaded;
    reloaded.load(copy);
    const auto copied_manifest = reloaded.get_source_manifest();
    const auto copied_it =
        std::find_if(copied_manifest.begin(), copied_manifest.end(), [&](const dip::SourceInfo& info) {
            return std::filesystem::path(info.path).filename() == source_file.filename();
        });
    ASSERT_NE(copied_it, copied_manifest.end());
    const auto& copied_source = *copied_it;
    EXPECT_EQ((copy_dir / copied_source.path).lexically_normal(), source_file);
    std::filesystem::remove_all(root);
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
    parser.add_schema_string("settings", "?descr \"Reusable settings\"\n?doi \"10.1234/settings\"\nvalue int = 42\n");
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
        EXPECT_EQ(minor_version, 11);
        ASSERT_GT(H5Lexists(hdf5_file, "/_DIPL_Sources", H5P_DEFAULT), 0);
        ASSERT_GT(H5Lexists(hdf5_file, "/_DIPL_Trace", H5P_DEFAULT), 0);
        ASSERT_GT(H5Lexists(hdf5_file, "/_DIPL_Dependencies", H5P_DEFAULT), 0);
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
        "$unit custom_length = 2*m\n"
        "$unit custom_span = 3*custom_length\n"
        "box_size float = 7500 custom_span\n"
    );
    const dip::Environment source = parser.parse();
    const auto file = environment_file("custom-units");
    source.save(file);

    dip::Environment loaded;
    loaded.load(file);
    EXPECT_DOUBLE_EQ(loaded["box_size"].as<double>(), 7500.0);
    EXPECT_EQ(loaded.get_node("box_size")->units->to_string(), "custom_span");
    const dip::EnvUnit& length = loaded.units.at("custom_length");
    const dip::EnvUnit& span = loaded.units.at("custom_span");
    EXPECT_EQ(length.definition, "2*m");
    EXPECT_EQ(span.definition, "3*custom_length");
    EXPECT_LT(length.registration_order, span.registration_order);

    const auto resaved = environment_file("custom-units-resaved");
    loaded.save(resaved);
    dip::Environment round_tripped;
    round_tripped.load(resaved);
    EXPECT_EQ(round_tripped.units.at("custom_span").id, span.id);
    EXPECT_EQ(round_tripped.units.at("custom_span").definition, span.definition);
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
