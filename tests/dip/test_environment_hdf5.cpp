#include "pch_tests.h"
#include "test_environment_fixture.h"
#include "../../src/snt/dip/hdf5/io_private.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <hdf5.h>
#include <limits>
#include <snt/dip/artifact.h>
#include <snt/dip/cursor.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/experiment.h>
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

    void replace_string_attribute(hid_t object, const char* name, const std::string& value) {
        if (H5Aexists(object, name) > 0)
            ASSERT_GE(H5Adelete(object, name), 0);
        H5Handle type(H5Tcopy(H5T_C_S1), H5Tclose);
        ASSERT_GE(H5Tset_size(type, value.size() + 1), 0);
        ASSERT_GE(H5Tset_cset(type, H5T_CSET_UTF8), 0);
        H5Handle space(H5Screate(H5S_SCALAR), H5Sclose);
        H5Handle attribute(H5Acreate2(object, name, type, space, H5P_DEFAULT, H5P_DEFAULT), H5Aclose);
        ASSERT_GE(H5Awrite(attribute, type, value.c_str()), 0);
    }

    void copy_fixed_attributes(hid_t source, hid_t destination) {
        const int count = H5Aget_num_attrs(source);
        ASSERT_GE(count, 0);
        for (int i = 0; i < count; ++i) {
            H5Handle attr(H5Aopen_idx(source, static_cast<unsigned>(i)), H5Aclose);
            const auto length = H5Aget_name(attr, 0, nullptr);
            ASSERT_GE(length, 0);
            std::string name(static_cast<size_t>(length) + 1, '\0');
            ASSERT_GE(H5Aget_name(attr, name.size(), name.data()), 0);
            name.resize(static_cast<size_t>(length));
            H5Handle type(H5Aget_type(attr), H5Tclose);
            H5Handle space(H5Aget_space(attr), H5Sclose);
            ASSERT_EQ(H5Tis_variable_str(type), 0);
            const auto count_values = H5Sget_simple_extent_npoints(space);
            ASSERT_GE(count_values, 0);
            std::vector<char> buffer(static_cast<size_t>(count_values) * H5Tget_size(type));
            H5Handle copy(H5Acreate2(destination, name.c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT), H5Aclose);
            ASSERT_GE(H5Aread(attr, type, buffer.data()), 0);
            ASSERT_GE(H5Awrite(copy, type, buffer.data()), 0);
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

    std::filesystem::remove(file);
    std::filesystem::remove(round_trip);
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
        "small_count int8 = -128\n"
        "gain float32 = 1.23456789\n"
        "matrix float64[2,2] = [[1.25, -2.5], [3.75, 4.5]]\n"
    );
    const auto original = parser.parse();
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto file = std::filesystem::temp_directory_path() / ("snt-numeric-fidelity-" + suffix + ".diph5");
    original.save(file);

    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        for (const auto& item : std::vector<std::pair<const char*, size_t>>{{"small_count", 1}, {"gain", 4}, {"matrix", 8},
                                                                            {"large_count", 8}}) {
            const auto path = std::string("/configuration/resolved/") + item.first;
            H5Handle dataset(H5Dopen2(archive, path.c_str(), H5P_DEFAULT), H5Dclose);
            H5Handle type(H5Dget_type(dataset), H5Tclose);
            EXPECT_EQ(H5Tget_size(type), item.second) << item.first;
        }
    }

    dip::Environment loaded;
    loaded.load(file);
    EXPECT_EQ(loaded["large_count"].as<uint64_t>(), uint64_t{9007199254740993ULL});
    EXPECT_EQ(loaded["maximum_count"].as<uint64_t>(), std::numeric_limits<uint64_t>::max());
    EXPECT_EQ(loaded["minimum_offset"].as<int64_t>(), std::numeric_limits<int64_t>::min());
    EXPECT_EQ(loaded["small_count"].as<int64_t>(), -128);
    EXPECT_EQ(loaded["small_count"].get_node()->value->get_dtype(), core::DataType::Integer8);
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
        H5Handle dataset(H5Dopen2(h5file, "/configuration/resolved/answer", H5P_DEFAULT), H5Dclose);
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
        // Keep this test focused on history validation, past the outer digest check.
        H5Handle provenance(H5Gopen2(h5file, "/configuration/provenance", H5P_DEFAULT), H5Gclose);
        replace_string_attribute(provenance, "fingerprint_status", "unavailable");
        replace_string_attribute(provenance, "configuration_sha256", "");
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
        H5Handle dataset(H5Dopen2(h5file, "/configuration/resolved/answer", H5P_DEFAULT), H5Dclose);
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
        H5Handle provenance(H5Gopen2(h5file, "/configuration/provenance", H5P_DEFAULT), H5Gclose);
        replace_string_attribute(provenance, "fingerprint_status", "unavailable");
        replace_string_attribute(provenance, "configuration_sha256", "");
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
        H5Handle event(H5Gopen2(h5file, "/configuration/provenance/dependencies/0", H5P_DEFAULT), H5Gclose);
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
    EXPECT_EQ(loaded.get_source_manifest().size(), loaded.archived_sources().size());
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

TEST(Environment, Diph5V3SourceArchiveRoundTripAndIntegrity) {
    const std::string source = "$override\n  radius = 20 cm\nradius float = 10 cm\n";
    dip::DIP parser;
    parser.add_string(source);
    const auto original = parser.parse();
    const auto file = environment_file("v3-source-archive");
    original.save(file);
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(static_cast<hid_t>(archive), 0);
        EXPECT_GT(H5Lexists(archive, "/configuration/resolved/radius", H5P_DEFAULT), 0);
        EXPECT_GT(H5Lexists(archive, "/model/sources/000000/content", H5P_DEFAULT), 0);
        EXPECT_GT(H5Lexists(archive, "/configuration/overrides/000000/content", H5P_DEFAULT), 0);
        EXPECT_EQ(H5Lexists(archive, "/execution", H5P_DEFAULT), 0);
    }
    dip::Environment loaded;
    loaded.load(file);
    EXPECT_EQ(loaded["radius"].as<double>(), 20);
    ASSERT_EQ(loaded.archived_sources().size(), 1);
    EXPECT_EQ(loaded.archived_sources()[0].content, source);
    ASSERT_EQ(loaded.applied_override_sources().size(), 1);
    EXPECT_EQ(loaded.applied_override_sources()[0].content, "$override\n  radius = 20 cm\n");

    const auto directory = std::filesystem::temp_directory_path() / "scinumtools3-v3-extracted";
    std::filesystem::create_directories(directory);
    loaded.extract_sources(directory);
    std::ifstream extracted(directory / "000000.dip", std::ios::binary);
    EXPECT_EQ(std::string(std::istreambuf_iterator<char>(extracted), {}), source);
    extracted.close();
    std::filesystem::remove(directory / "000000.dip");
    std::filesystem::remove(directory);

    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        H5Handle bytes(H5Dopen2(archive, "/model/sources/000000/content", H5P_DEFAULT), H5Dclose);
        H5Handle space(H5Dget_space(bytes), H5Sclose);
        hsize_t start = 0, count = 1;
        ASSERT_GE(H5Sselect_hyperslab(space, H5S_SELECT_SET, &start, nullptr, &count, nullptr), 0);
        H5Handle memory(H5Screate_simple(1, &count, nullptr), H5Sclose);
        const unsigned char replacement = 'X';
        ASSERT_GE(H5Dwrite(bytes, H5T_NATIVE_UINT8, memory, space, H5P_DEFAULT, &replacement), 0);
    }
    dip::Environment corrupted;
    EXPECT_THROW(corrupted.load(file), dip::IOException);
    std::filesystem::remove(file);
}

TEST(Environment, Diph5V3PreservesResolvedHierarchyAndOrderedOverrides) {
    // The regular hierarchy fixture exercises collections, arrays, metadata, and units.
    const auto hierarchy_file = environment_file("v3-hierarchy");
    parsed_environment().save(hierarchy_file);
    dip::Environment hierarchy;
    hierarchy.load(hierarchy_file);
    EXPECT_EQ(hierarchy["simulation.steps"].as<int64_t>(), 100);
    EXPECT_EQ(hierarchy["samples[1].time"].as<double>(), 1.0);
    EXPECT_EQ(hierarchy.get_node("title")->metadata.description, "Environment round-trip fixture");
    std::filesystem::remove(hierarchy_file);

    dip::DIP parser;
    parser.add_string("first int = 1\nsecond int = 1\n");
    parser.add_override_string("first = 2\n", "first_override");
    parser.add_override_string("second = 3\n", "second_override");
    const auto file = environment_file("v3-ordered-overrides");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    EXPECT_EQ(loaded["first"].as<int64_t>(), 2);
    EXPECT_EQ(loaded["second"].as<int64_t>(), 3);
    ASSERT_EQ(loaded.applied_override_sources().size(), 2);
    EXPECT_EQ(loaded.applied_override_sources()[0].name, "first_override");
    EXPECT_EQ(loaded.applied_override_sources()[0].content, "first = 2\n");
    EXPECT_EQ(loaded.applied_override_sources()[1].name, "second_override");
    EXPECT_EQ(loaded.applied_override_sources()[1].content, "second = 3\n");
    std::filesystem::remove(file);
}

TEST(Environment, Diph5V3RejectsMalformedArchiveEntries) {
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    parser.add_override_string("answer = 8\n", "answer_override");
    const auto original = environment_file("v3-adversarial-base");
    parser.parse().save(original);

    const auto rejects = [&](const std::string& label, const auto& damage) {
        const auto file = environment_file("v3-adversarial-" + label);
        std::filesystem::copy_file(original, file, std::filesystem::copy_options::overwrite_existing);
        {
            H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
            ASSERT_GE(static_cast<hid_t>(archive), 0);
            damage(archive);
        }
        dip::Environment loaded;
        EXPECT_THROW(loaded.load(file), dip::IOException) << label;
        std::filesystem::remove(file);
    };

    rejects("missing-version", [](hid_t archive) {
        ASSERT_GE(H5Adelete(archive, "_DIPH5_Schema_Minor"), 0);
    });
    rejects("missing-resolved", [](hid_t archive) {
        ASSERT_GE(H5Ldelete(archive, "/configuration/resolved", H5P_DEFAULT), 0);
    });
    rejects("missing-bytes", [](hid_t archive) {
        ASSERT_GE(H5Ldelete(archive, "/model/sources/000000/content", H5P_DEFAULT), 0);
    });
    rejects("bad-digest", [](hid_t archive) {
        H5Handle source(H5Gopen2(archive, "/model/sources/000000", H5P_DEFAULT), H5Gclose);
        replace_string_attribute(source, "sha256", std::string(64, '0'));
    });
    rejects("duplicate-id", [](hid_t archive) {
        ASSERT_GE(H5Ocopy(archive, "/model/sources/000000", archive, "/model/sources/000001",
                          H5P_DEFAULT, H5P_DEFAULT), 0);
        H5Handle manifest(H5Gopen2(archive, "/model/manifest", H5P_DEFAULT), H5Gclose);
        H5Handle count(H5Aopen(manifest, "source_count", H5P_DEFAULT), H5Aclose);
        const uint64_t two = 2;
        ASSERT_GE(H5Awrite(count, H5T_NATIVE_UINT64, &two), 0);
    });
    rejects("broken-parent", [](hid_t archive) {
        H5Handle source(H5Gopen2(archive, "/model/sources/000000", H5P_DEFAULT), H5Gclose);
        replace_string_attribute(source, "parent_source_id", "missing-source");
        H5Handle line(H5Aopen(source, "parent_line", H5P_DEFAULT), H5Aclose);
        const uint64_t one = 1;
        ASSERT_GE(H5Awrite(line, H5T_NATIVE_UINT64, &one), 0);
    });
    rejects("invalid-utf8", [](hid_t archive) {
        H5Handle source(H5Gopen2(archive, "/model/sources/000000", H5P_DEFAULT), H5Gclose);
        replace_string_attribute(source, "name", std::string("bad\xff", 4));
    });
    rejects("invalid-metadata-utf8", [](hid_t archive) {
        H5Handle source(H5Gopen2(archive, "/model/sources/000000", H5P_DEFAULT), H5Gclose);
        replace_string_attribute(source, "description", std::string("bad\xff", 4));
    });
    rejects("array-attribute", [](hid_t archive) {
        H5Handle source(H5Gopen2(archive, "/model/sources/000000", H5P_DEFAULT), H5Gclose);
        ASSERT_GE(H5Adelete(source, "name"), 0);
        H5Handle type(H5Tcopy(H5T_C_S1), H5Tclose);
        ASSERT_GE(H5Tset_size(type, 8), 0);
        const hsize_t count = 2;
        H5Handle space(H5Screate_simple(1, &count, nullptr), H5Sclose);
        H5Handle attribute(H5Acreate2(source, "name", type, space, H5P_DEFAULT, H5P_DEFAULT), H5Aclose);
        const char names[16] = "first\0second";
        ASSERT_GE(H5Awrite(attribute, type, names), 0);
    });
    rejects("override-order", [](hid_t archive) {
        H5Handle entry(H5Gopen2(archive, "/configuration/overrides/000000", H5P_DEFAULT), H5Gclose);
        H5Handle order(H5Aopen(entry, "order", H5P_DEFAULT), H5Aclose);
        const uint64_t one = 1;
        ASSERT_GE(H5Awrite(order, H5T_NATIVE_UINT64, &one), 0);
    });
    rejects("override-reference", [](hid_t archive) {
        H5Handle entry(H5Gopen2(archive, "/configuration/overrides/000000", H5P_DEFAULT), H5Gclose);
        replace_string_attribute(entry, "source_id", "missing-source");
    });
    rejects("node-order", [](hid_t archive) {
        H5Handle node(H5Oopen(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Oclose);
        H5Handle order(H5Aopen(node, "_DIPL_Order", H5P_DEFAULT), H5Aclose);
        const uint64_t one = 1;
        ASSERT_GE(H5Awrite(order, H5T_NATIVE_UINT64, &one), 0);
    });
    rejects("node-source-reference", [](hid_t archive) {
        H5Handle node(H5Oopen(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Oclose);
        replace_string_attribute(node, "_DIPL_Source", "missing-source");
    });
    rejects("node-path-mismatch", [](hid_t archive) {
        H5Handle node(H5Oopen(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Oclose);
        replace_string_attribute(node, "_DIPL_Path", "other");
    });
    rejects("node-dtype-mismatch", [](hid_t archive) {
        H5Handle node(H5Oopen(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Oclose);
        replace_string_attribute(node, "_DIPL_Value_Type", "int8");
    });
    rejects("node-invalid-kind", [](hid_t archive) {
        H5Handle node(H5Oopen(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Oclose);
        replace_string_attribute(node, "_DIPL_Kind", "invalid");
    });
    rejects("node-invalid-node-type", [](hid_t archive) {
        H5Handle node(H5Oopen(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Oclose);
        replace_string_attribute(node, "_DIPL_Node_Type", "invalid");
    });
    rejects("scalar-array-marker", [](hid_t archive) {
        H5Handle node(H5Oopen(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Oclose);
        H5Handle marker(H5Aopen(node, "_DIPL_Array", H5P_DEFAULT), H5Aclose);
        const uint8_t one = 1;
        ASSERT_GE(H5Awrite(marker, H5T_NATIVE_UINT8, &one), 0);
    });
    rejects("unexpected-execution", [](hid_t archive) {
        H5Handle group(H5Gcreate2(archive, "/execution", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose);
        ASSERT_GE(static_cast<hid_t>(group), 0);
    });
    rejects("missing-unit-manifest", [](hid_t archive) {
        ASSERT_GE(H5Ldelete(archive, "/model/registrations/units", H5P_DEFAULT), 0);
    });
    std::filesystem::remove(original);
}

TEST(Environment, Diph5V3ExtractionIgnoresOriginalPaths) {
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    const auto file = environment_file("v3-untrusted-path");
    parser.parse().save(file);
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        H5Handle source(H5Gopen2(archive, "/model/sources/000000", H5P_DEFAULT), H5Gclose);
        replace_string_attribute(source, "original_path", "../../outside.dip");
    }
    dip::Environment loaded;
    loaded.load(file);
    const auto destination = environment_file("v3-extract-directory");
    std::filesystem::create_directories(destination);
    const auto safe_file = destination / "000000.dip";
    loaded.extract_sources(destination);
    EXPECT_TRUE(std::filesystem::is_regular_file(safe_file));
    EXPECT_ANY_THROW(loaded.extract_sources(destination));
    std::filesystem::remove(safe_file);
    std::filesystem::remove(destination);
    std::filesystem::remove(file);
}

TEST(Environment, Diph5V3RejectsUnterminatedStringData) {
    dip::DIP parser;
    parser.add_string("label str = \"abc\"\n");
    const auto file = environment_file("v3-unterminated-string");
    parser.parse().save(file);
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        H5Handle dataset(H5Dopen2(archive, "/configuration/resolved/label", H5P_DEFAULT), H5Dclose);
        H5Handle type(H5Dget_type(dataset), H5Tclose);
        ASSERT_EQ(H5Tget_size(type), 4);
        const char bytes[] = {'a', 'b', 'c', 'd'};
        ASSERT_GE(H5Dwrite(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, bytes), 0);
    }
    dip::Environment loaded;
    EXPECT_THROW(loaded.load(file), dip::IOException);
    std::filesystem::remove(file);
}

TEST(Environment, Diph5V3ArchivesImportedSourceBytesInOrder) {
    const auto directory = environment_file("v3-imports");
    std::filesystem::create_directories(directory);
    const std::string leaf = "value int = 42\n# keep this comment\n";
    {
        std::ofstream main(directory / "main.dip", std::ios::binary);
        main << "$source left = \"left.dip\"\n"
                "$source right = \"right.dip\"\n"
                "answer int = {left?value}\n";
        std::ofstream left(directory / "left.dip", std::ios::binary);
        left << leaf;
        std::ofstream right(directory / "right.dip", std::ios::binary);
        right << leaf;
    }
    dip::DIP parser;
    parser.add_file(directory / "main.dip");
    const auto file = environment_file("v3-imports-archive");
    parser.parse().save(file);
    dip::Environment loaded;
    loaded.load(file);
    EXPECT_EQ(loaded["answer"].as<int64_t>(), 42);
    ASSERT_EQ(loaded.archived_sources().size(), 3);
    EXPECT_FALSE(loaded.archived_sources()[0].info.name.empty());
    EXPECT_EQ(loaded.archived_sources()[1].info.name, "left");
    EXPECT_EQ(loaded.archived_sources()[2].info.name, "right");
    EXPECT_EQ(loaded.archived_sources()[1].content, leaf);
    EXPECT_EQ(loaded.archived_sources()[2].content, leaf);
    EXPECT_EQ(loaded.archived_sources()[1].info.parent_name, loaded.archived_sources()[0].source_id);
    EXPECT_EQ(loaded.archived_sources()[2].info.parent_name, loaded.archived_sources()[0].source_id);
    std::filesystem::remove(file);
    std::filesystem::remove_all(directory);
}

TEST(Environment, Diph5V3ExperimentLifecycleAndImport) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    const auto archive = environment_file("v3-experiment-" + suffix);
    auto experiment = dip::Experiment::prepare(parser.parse(), archive);
    EXPECT_EQ(experiment.outcome().status, dip::ExperimentStatus::Prepared);
    EXPECT_ANY_THROW(experiment.finish(dip::ExperimentStatus::Completed, 0));

    dip::ExperimentCommand command;
    command.argv = {"echo", "two words", "", "a\"b", "μ"};
    command.launcher_argv = {"launcher", "--label", "a b"};
    command.argv_redacted = true;
    command.working_directory = std::filesystem::current_path();
    experiment.start(command);
    const auto running = dip::Experiment::open(archive).outcome();
    EXPECT_EQ(running.status, dip::ExperimentStatus::Running);
    ASSERT_TRUE(running.command.has_value());
    EXPECT_EQ(running.command->argv, command.argv);
    EXPECT_EQ(running.command->launcher_argv, command.launcher_argv);
    EXPECT_TRUE(running.command->argv_redacted);
    EXPECT_EQ(running.command->capture_origin, "launcher_observed");
    EXPECT_ANY_THROW(experiment.start(command));
    experiment.finish(dip::ExperimentStatus::Completed, 0);
    const auto completed = experiment.outcome();
    EXPECT_EQ(completed.status, dip::ExperimentStatus::Completed);
    EXPECT_EQ(completed.exit_code, 0);
    EXPECT_FALSE(completed.finished_at.empty());
    EXPECT_ANY_THROW(experiment.finish(dip::ExperimentStatus::Failed, 1));
    dip::Environment loaded;
    loaded.load(archive);
    EXPECT_EQ(loaded["answer"].as<int64_t>(), 7);
    {
        H5Handle file(H5Fopen(archive.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        replace_string_attribute(file, "_DIPH5_State", "running");
    }
    EXPECT_ANY_THROW(dip::Experiment::open(archive));
    std::filesystem::remove(archive);

    const auto environment_file_path = environment_file("v3-environment-for-import-" + suffix);
    dip::DIP second;
    second.add_string("answer int = 7\n");
    second.parse().save(environment_file_path);
    const auto host_file = environment_file("v3-host-experiment-" + suffix);
    auto host = dip::Experiment::prepare_from_archive(environment_file_path, host_file);
    command.capture_origin = "host_reported";
    command.reported_executable_path = running.resolved_executable_path;
    command.reported_executable_sha256 = running.executable_sha256;
    command.reported_executable_size_bytes = running.executable_size_bytes;
    host.start(command);
    host.finish(dip::ExperimentStatus::Completed, 0);
    const auto reported = host.outcome();
    ASSERT_TRUE(reported.command.has_value());
    EXPECT_EQ(reported.command->argv, running.command->argv);
    EXPECT_EQ(reported.command->working_directory, running.command->working_directory);
    EXPECT_EQ(reported.command->launcher_argv, running.command->launcher_argv);
    EXPECT_EQ(reported.command->argv_redacted, running.command->argv_redacted);
    EXPECT_EQ(reported.command->command_role, running.command->command_role);
    EXPECT_EQ(reported.executable_sha256, running.executable_sha256);
    EXPECT_EQ(reported.executable_size_bytes, running.executable_size_bytes);
    EXPECT_EQ(reported.resolved_executable_path, running.resolved_executable_path);
    EXPECT_EQ(reported.status, completed.status);
    EXPECT_EQ(reported.exit_code, completed.exit_code);
    EXPECT_EQ(reported.capture_origin, "host_reported");
    EXPECT_TRUE(reported.system_os.empty());
    EXPECT_TRUE(reported.system_architecture.empty());
    EXPECT_TRUE(reported.started_at.empty());
    EXPECT_TRUE(reported.finished_at.empty());
    std::filesystem::remove(host_file);

    const auto interrupted_file = environment_file("v3-interrupted-" + suffix);
    auto interrupted = dip::Experiment::prepare_from_archive(environment_file_path, interrupted_file);
    interrupted.start(command);
    auto reopened = dip::Experiment::open(interrupted_file);
    EXPECT_EQ(reopened.outcome().status, dip::ExperimentStatus::Running);
    EXPECT_ANY_THROW(reopened.import(dip::ExperimentStatus::Completed, 0));
    reopened.finish(dip::ExperimentStatus::Aborted, {}, "cancelled");
    EXPECT_EQ(reopened.outcome().status, dip::ExperimentStatus::Aborted);
    EXPECT_ANY_THROW(reopened.finish(dip::ExperimentStatus::Completed, 0));
    std::filesystem::remove(interrupted_file);

    const auto imported_file = environment_file("v3-imported-experiment-" + suffix);
    auto imported = dip::Experiment::prepare_from_archive(environment_file_path, imported_file);
    imported.import(dip::ExperimentStatus::Failed, 42, "External scheduler reported failure");
    const auto outcome = imported.outcome();
    EXPECT_EQ(outcome.status, dip::ExperimentStatus::Failed);
    EXPECT_EQ(outcome.capture_origin, "imported");
    EXPECT_TRUE(outcome.started_at.empty());
    EXPECT_TRUE(outcome.finished_at.empty());
    EXPECT_FALSE(outcome.command.has_value());
    EXPECT_EQ(outcome.exit_code, 42);
    EXPECT_ANY_THROW(imported.start(command));
    std::filesystem::remove(imported_file);
    const auto completed_file = environment_file("v3-imported-completed-" + suffix);
    auto imported_completed = dip::Experiment::prepare_from_archive(environment_file_path, completed_file);
    imported_completed.import(dip::ExperimentStatus::Completed, 0);
    EXPECT_EQ(imported_completed.outcome().status, dip::ExperimentStatus::Completed);
    EXPECT_TRUE(imported_completed.results().empty());
    EXPECT_EQ(imported_completed.verify().integrity, "pass");
    std::filesystem::remove(completed_file);
    std::filesystem::remove(environment_file_path);
}

TEST(Environment, Diph5V3PrepareDoesNotReplaceExistingFile) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto input = environment_file("v3-prepare-input-" + suffix);
    const auto output = environment_file("v3-prepare-existing-" + suffix);
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    const auto environment = parser.parse();
    environment.save(input);
    { std::ofstream marker(output, std::ios::binary); marker << "existing"; }
    EXPECT_ANY_THROW(dip::Experiment::prepare(environment, output));
    EXPECT_ANY_THROW(dip::Experiment::prepare_from_archive(input, output));
    std::ifstream marker(output, std::ios::binary);
    EXPECT_EQ(std::string(std::istreambuf_iterator<char>(marker), {}), "existing");
    marker.close();
    std::filesystem::remove(input);
    std::filesystem::remove(output);
}

TEST(Environment, Diph5V3ResultsFinalizeVerifyAndPreserveFailure) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto archive = environment_file("v3-results-" + suffix);
    const auto artifact = archive.parent_path() / ("result-" + suffix + ".nc");
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    auto experiment = dip::Experiment::prepare(parser.parse(), archive);
    dip::ExperimentCommand command;
    command.argv = {"simulation"};
    command.working_directory = std::filesystem::current_path();
    experiment.start(command);
    const auto scalar = experiment.add_dataset("energy/J", {42.5}, {}, "J");
    const auto array = experiment.add_dataset("spectrum", {1, 2, 3, 4}, {2, 2}, "m", {"row", "column"});
    const auto diagnostic = experiment.add_diagnostic("NaN count", 0, "observed");
    { std::ofstream out(artifact, std::ios::binary); out << "CDF\0data"; }
    const auto attached = experiment.attach_artifact(artifact, "output.nc", "NetCDF");
    const auto missing = experiment.attach_artifact(artifact.parent_path() / "absent-result.nc", "missing");
    const auto results = experiment.results();
    ASSERT_EQ(results.size(), 5);
    EXPECT_NE(scalar, array);
    EXPECT_NE(array, diagnostic);
    EXPECT_NE(attached, missing);
    const auto find_result = [&](const std::string& id) {
        return std::find_if(results.begin(), results.end(), [&](const auto& item) { return item.result_id == id; });
    };
    EXPECT_EQ(find_result(scalar)->values, (std::vector<double>{42.5}));
    EXPECT_EQ(find_result(array)->shape, (std::vector<uint64_t>{2, 2}));
    // Calculated independently using Python hashlib and struct, from the published
    // diph5-result-v1 byte encoding.
    EXPECT_EQ(find_result(array)->sha256, "cccf3bbbfabfcd462a458786e248fb8c1dd44a9c4a429c0d73cb64038a9cca5a");
    EXPECT_EQ(find_result(diagnostic)->verification, "observed");
    EXPECT_EQ(experiment.verify().external_artifacts, "unavailable");
    // Repack the data with chunking and compression. The digest covers logical
    // values, not the physical HDF5 representation.
    {
        H5Handle file(H5Fopen(archive.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        const auto path = "/results/datasets/" + array + "/data";
        ASSERT_GE(H5Ldelete(file, path.c_str(), H5P_DEFAULT), 0);
        const hsize_t shape[] = {2, 2};
        const hsize_t chunks[] = {1, 2};
        H5Handle space(H5Screate_simple(2, shape, nullptr), H5Sclose);
        H5Handle properties(H5Pcreate(H5P_DATASET_CREATE), H5Pclose);
        ASSERT_GE(H5Pset_chunk(properties, 2, chunks), 0);
        ASSERT_GE(H5Pset_deflate(properties, 1), 0);
        H5Handle data(H5Dcreate2(file, path.c_str(), H5T_IEEE_F64LE, space, H5P_DEFAULT, properties, H5P_DEFAULT), H5Dclose);
        const double values[] = {1, 2, 3, 4};
        ASSERT_GE(H5Dwrite(data, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, values), 0);
    }
    EXPECT_EQ(experiment.verify().integrity, "pass");
    experiment.finalize_results();
    EXPECT_ANY_THROW(experiment.add_dataset("late", {1}, {}));
    experiment.finish(dip::ExperimentStatus::Completed, 0);
    EXPECT_EQ(experiment.verify().integrity, "pass");
    EXPECT_EQ(dip::Experiment::open(archive).results().size(), 5);
    { std::ofstream out(artifact, std::ios::binary | std::ios::trunc); out << "changed"; }
    EXPECT_EQ(experiment.verify().external_artifacts, "fail");
    {
        H5Handle file(H5Fopen(archive.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        const auto dataset_path = "/results/datasets/" + scalar + "/data";
        H5Handle data(H5Dopen2(file, dataset_path.c_str(), H5P_DEFAULT), H5Dclose);
        const double damaged = 0;
        ASSERT_GE(H5Dwrite(data, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &damaged), 0);
    }
    EXPECT_EQ(experiment.verify().structural, "pass");
    EXPECT_EQ(experiment.verify().integrity, "fail");
    std::filesystem::remove(artifact);
    std::filesystem::remove(archive);

    const auto failed_file = environment_file("v3-failed-results-" + suffix);
    dip::DIP second;
    second.add_string("answer int = 7\n");
    auto failed = dip::Experiment::prepare(second.parse(), failed_file);
    failed.start(command);
    failed.add_dataset("partial", {1}, {});
    failed.finish(dip::ExperimentStatus::Failed, 1);
    EXPECT_EQ(failed.results().size(), 1);
    EXPECT_EQ(failed.verify().integrity, "pass");
    std::filesystem::remove(failed_file);
}

TEST(Environment, Diph5V3CannotSealOrCompleteChangedArtifact) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto archive = environment_file("v3-changing-result-" + suffix);
    const auto artifact = archive.parent_path() / ("changing-result-" + suffix);
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    auto experiment = dip::Experiment::prepare(parser.parse(), archive);
    dip::ExperimentCommand command;
    command.argv = {"simulation"};
    command.working_directory = std::filesystem::current_path();
    experiment.start(command);
    { std::ofstream out(artifact); out << "original"; }
    experiment.attach_artifact(artifact, "output");
    { std::ofstream out(artifact); out << "changed"; }
    EXPECT_ANY_THROW(experiment.finalize_results());
    EXPECT_EQ(experiment.outcome().status, dip::ExperimentStatus::Running);
    { std::ofstream out(artifact); out << "original"; }
    experiment.finalize_results();
    { std::ofstream out(artifact); out << "changed"; }
    EXPECT_ANY_THROW(experiment.finish(dip::ExperimentStatus::Completed, 0));
    EXPECT_EQ(experiment.outcome().status, dip::ExperimentStatus::Running);
    experiment.finish(dip::ExperimentStatus::Failed, 1);
    std::filesystem::remove(artifact);
    std::filesystem::remove(archive);
}

TEST(Environment, Diph5V3TypedResultStorageAndBounds) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto archive = environment_file("v3-typed-results-" + suffix);
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    auto experiment = dip::Experiment::prepare(parser.parse(), archive);
    dip::ExperimentCommand command;
    command.argv = {"simulation"};
    command.working_directory = std::filesystem::current_path();
    experiment.start(command);
    for (const auto& dtype : {"int8", "int16", "int32", "int64", "char"})
        experiment.add_typed_dataset(dtype, std::vector<int64_t>{-1, 42}, dtype, {2});
    for (const auto& dtype : {"uint8", "uint16", "uint32", "uint64", "bool"})
        experiment.add_typed_dataset(dtype, std::vector<uint64_t>{0, 1}, dtype, {2});
    experiment.add_typed_dataset("maximum", std::vector<uint64_t>{UINT64_MAX}, "uint64", {});
    experiment.add_typed_dataset("minimum", std::vector<int64_t>{INT64_MIN}, "int64", {});
    for (const auto& dtype : {"float32", "float64"})
        experiment.add_typed_dataset(dtype, std::vector<double>{1.5, -2.25}, dtype, {2});
    EXPECT_ANY_THROW(experiment.add_typed_dataset("bad", std::vector<int64_t>{128}, "int8", {}));
    EXPECT_ANY_THROW(experiment.add_typed_dataset("bad", std::vector<uint64_t>{256}, "uint8", {}));
    EXPECT_ANY_THROW(experiment.add_typed_dataset("bad", std::vector<uint64_t>{2}, "bool", {}));
    EXPECT_ANY_THROW(experiment.add_typed_dataset("bad", std::vector<double>{1}, "int64", {}));
    EXPECT_ANY_THROW(experiment.add_typed_dataset("bad", std::vector<double>{1}, "float128", {}));
    const auto results = experiment.results();
    ASSERT_EQ(results.size(), 14);
    for (const auto& item : results) {
        if (item.name == "maximum") EXPECT_EQ(std::get<std::vector<uint64_t>>(item.typed_values)[0], UINT64_MAX);
        if (item.name == "minimum") EXPECT_EQ(std::get<std::vector<int64_t>>(item.typed_values)[0], INT64_MIN);
        if (item.name == "float32") EXPECT_EQ(item.logical_dtype, "float32");
    }
    experiment.finish(dip::ExperimentStatus::Completed, 0);
    EXPECT_EQ(experiment.verify().integrity, "pass");
    std::filesystem::remove(archive);
}

TEST(Environment, Diph5V3LargeResultRoundTripAndIntegrity) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto archive = environment_file("v3-large-result-" + suffix);
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    auto experiment = dip::Experiment::prepare(parser.parse(), archive);
    dip::ExperimentCommand command;
    command.argv = {"simulation"};
    command.working_directory = std::filesystem::current_path();
    experiment.start(command);
    std::vector<double> values(1024 * 1024);
    for (size_t index = 0; index < values.size(); ++index) values[index] = static_cast<double>(index) / 8;
    experiment.add_dataset("large", values, {1024, 1024});
    experiment.finish(dip::ExperimentStatus::Completed, 0);
    const auto results = dip::Experiment::open(archive).results();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].shape, (std::vector<uint64_t>{1024, 1024}));
    EXPECT_EQ(results[0].values, values);
    EXPECT_EQ(experiment.verify().integrity, "pass");
    std::filesystem::remove(archive);
}

TEST(Environment, Diph5V3RejectsCorruptResultInventory) {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto original = environment_file("v3-result-adversarial-base-" + suffix);
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    auto experiment = dip::Experiment::prepare(parser.parse(), original);
    dip::ExperimentCommand command;
    command.argv = {"simulation"};
    command.working_directory = std::filesystem::current_path();
    experiment.start(command);
    const auto id = experiment.add_typed_dataset("count", std::vector<int64_t>{12}, "int8", {});
    experiment.finish(dip::ExperimentStatus::Completed, 0);
    const auto result_path = "/results/datasets/" + id;
    const auto rejects = [&](const std::string& label, const auto& damage) {
        const auto file = environment_file("v3-result-adversarial-" + label + suffix);
        std::filesystem::copy_file(original, file, std::filesystem::copy_options::overwrite_existing);
        {
            H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
            damage(archive);
        }
        EXPECT_ANY_THROW(dip::Experiment::open(file)) << label;
        std::filesystem::remove(file);
    };
    rejects("missing-entry", [&](hid_t archive) {
        const auto path = "/results/manifest/entries/" + id;
        ASSERT_GE(H5Ldelete(archive, path.c_str(), H5P_DEFAULT), 0);
    });
    rejects("wrong-owner", [&](hid_t archive) {
        H5Handle entry(H5Gopen2(archive, result_path.c_str(), H5P_DEFAULT), H5Gclose);
        replace_string_attribute(entry, "run_id", "different-run");
    });
    rejects("wrong-dtype", [&](hid_t archive) {
        H5Handle entry(H5Gopen2(archive, result_path.c_str(), H5P_DEFAULT), H5Gclose);
        replace_string_attribute(entry, "logical_dtype", "uint8");
    });
    rejects("unfinalized", [&](hid_t archive) {
        H5Handle manifest(H5Gopen2(archive, "/results/manifest", H5P_DEFAULT), H5Gclose);
        H5Handle attr(H5Aopen(manifest, "finalized", H5P_DEFAULT), H5Aclose);
        const uint8_t no = 0;
        ASSERT_GE(H5Awrite(attr, H5T_NATIVE_UINT8, &no), 0);
    });
    std::filesystem::remove(original);
}

TEST(Environment, RejectsDiph5Version2Archive) {
    const auto file = environment_file("reject-version-2");
    {
        H5Handle archive(H5Fcreate(file.string().c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(static_cast<hid_t>(archive), 0);
        replace_string_attribute(archive, "_DIPL_Format", "SciNumTools3 Environment");
        const uint64_t version = 2;
        H5Handle space(H5Screate(H5S_SCALAR), H5Sclose);
        H5Handle attribute(H5Acreate2(archive, "_DIPL_Schema_Version", H5T_NATIVE_UINT64,
                                     space, H5P_DEFAULT, H5P_DEFAULT), H5Aclose);
        ASSERT_GE(H5Awrite(attribute, H5T_NATIVE_UINT64, &version), 0);
    }
    dip::Environment loaded;
    EXPECT_THROW(loaded.load(file), dip::IOException);
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

TEST(Environment, Diph5V3FailedSaveKeepsPreviousArchive) {
    const auto file = environment_file("atomic-save");
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    auto environment = parser.parse();
    environment.save(file);
    environment.get_node("answer")->value_dtype = core::DataType::Boolean;
    EXPECT_ANY_THROW(environment.save(file));
    dip::Environment loaded;
    ASSERT_NO_THROW(loaded.load(file));
    EXPECT_EQ(loaded["answer"].as<int64_t>(), 7);
    std::filesystem::remove(file);
}

TEST(Environment, Diph5V3CandidateConfigurationFingerprintGoldenVector) {
    const auto file = environment_file("candidate-fingerprint");
    dip::DIP parser(dip::Source{"DIP0", 0});
    parser.add_string("answer int = 7\n");
    parser.parse().save(file);
    std::string original;
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        const auto digest = dip::hdf5::detail::configuration_fingerprint(archive);
        ASSERT_TRUE(digest.has_value());
        original = *digest;
        EXPECT_EQ(original, "b75c824d5e69cbbb6abed9f33adfc503cd0c347969087746044eb24dff64dcac");
    }
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        H5Handle data(H5Dopen2(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Dclose);
        replace_string_attribute(data, "_DIPL_Value_Type", "float128");
        EXPECT_FALSE(dip::hdf5::detail::configuration_fingerprint(archive).has_value());
        replace_string_attribute(data, "_DIPL_Value_Type", "int32");
        replace_string_attribute(data, "created", "2026-01-01T00:00:00Z");
        const auto timestamp_only = dip::hdf5::detail::configuration_fingerprint(archive);
        ASSERT_TRUE(timestamp_only.has_value());
        EXPECT_EQ(*timestamp_only, original);
        replace_string_attribute(data, "description", "Changed metadata");
        const auto changed_metadata = dip::hdf5::detail::configuration_fingerprint(archive);
        ASSERT_TRUE(changed_metadata.has_value());
        EXPECT_NE(*changed_metadata, original);
        ASSERT_GE(H5Adelete(data, "description"), 0);
        const int64_t changed = 8;
        ASSERT_GE(H5Dwrite(data, H5T_NATIVE_INT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, &changed), 0);
        const auto digest = dip::hdf5::detail::configuration_fingerprint(archive);
        ASSERT_TRUE(digest.has_value());
        EXPECT_NE(*digest, original);
    }
    std::filesystem::remove(file);
}

TEST(Environment, Diph5V3RejectsChangedResolvedValue) {
    const auto file = environment_file("fingerprint-integrity");
    dip::DIP parser;
    parser.add_string("answer int = 7\n");
    parser.parse().save(file);
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        H5Handle value(H5Dopen2(archive, "/configuration/resolved/answer", H5P_DEFAULT), H5Dclose);
        const int64_t changed = 8;
        ASSERT_GE(H5Dwrite(value, H5T_NATIVE_INT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, &changed), 0);
    }
    dip::Environment loaded;
    EXPECT_ANY_THROW(loaded.load(file));
    std::filesystem::remove(file);
}

TEST(Environment, Diph5V3CandidateConfigurationFingerprintRichVector) {
    const auto file = environment_file("candidate-fingerprint-rich");
    dip::DIP rich_parser(dip::Source{"DIP0", 0});
    rich_parser.add_string("a int8[2] = [-128, 127]\nb float32 = -0.0\nc str = \"μ\"\n");
    rich_parser.parse().save(file);
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        const auto digest = dip::hdf5::detail::configuration_fingerprint(archive);
        ASSERT_TRUE(digest.has_value());
        EXPECT_EQ(*digest, "a6fbef7eca599dacb5ae4aa9bb59269a2c1ed146fe409da84238573743d7bbff");
        {
            H5Handle group(H5Gopen2(archive, "/configuration/resolved", H5P_DEFAULT), H5Gclose);
            H5Handle old(H5Dopen2(group, "a", H5P_DEFAULT), H5Dclose);
            H5Handle space(H5Dget_space(old), H5Sclose);
            H5Handle type(H5Dget_type(old), H5Tclose);
            H5Handle properties(H5Pcreate(H5P_DATASET_CREATE), H5Pclose);
            const hsize_t chunks[] = {1};
            ASSERT_GE(H5Pset_chunk(properties, 1, chunks), 0);
            ASSERT_GE(H5Pset_deflate(properties, 1), 0);
            H5Handle packed(H5Dcreate2(group, "repacked", type, space, H5P_DEFAULT, properties, H5P_DEFAULT), H5Dclose);
            int8_t values[2];
            ASSERT_GE(H5Dread(old, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, values), 0);
            ASSERT_GE(H5Dwrite(packed, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, values), 0);
            copy_fixed_attributes(old, packed);
            ASSERT_GE(H5Ldelete(group, "a", H5P_DEFAULT), 0);
            ASSERT_GE(H5Lmove(group, "repacked", group, "a", H5P_DEFAULT, H5P_DEFAULT), 0);
        }
        const auto repacked_digest = dip::hdf5::detail::configuration_fingerprint(archive);
        ASSERT_TRUE(repacked_digest.has_value());
        EXPECT_EQ(*repacked_digest, *digest);
    }
    std::filesystem::remove(file);
}

TEST(Environment, Diph5V3CandidateConfigurationFingerprintHierarchyVector) {
    const auto file = environment_file("candidate-fingerprint-hierarchy");
    dip::DIP parser(dip::Source{"DIP0", 0});
    parser.add_string("physics\n  radius float64 = 4 m\n  limit int16 = 6\n"
                      "catalog map\ncatalog[alpha]\n  label str = \"x\"\n");
    parser.parse().save(file);
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        const auto digest = dip::hdf5::detail::configuration_fingerprint(archive);
        ASSERT_TRUE(digest.has_value());
        EXPECT_EQ(*digest, "70e39d758705403cace4b6c6c7c4a36b1f46a4d548950b39ba23d526f65a4b5f");
    }
    std::filesystem::remove(file);
}

TEST(Environment, SaveHdf5Content) {
    const auto file = environment_file("save-content");
    parsed_environment().save(file);
    {
        H5Handle archive(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        ASSERT_GE(static_cast<hid_t>(archive), 0);
        EXPECT_GT(H5Aexists(archive, "_DIPH5_Format"), 0);
        EXPECT_EQ(H5Aexists(archive, "_DIPL_Format"), 0);
        H5Handle major_attribute(H5Aopen(archive, "_DIPH5_Schema_Major", H5P_DEFAULT), H5Aclose);
        uint32_t version = 0;
        ASSERT_GE(H5Aread(major_attribute, H5T_NATIVE_UINT32, &version), 0);
        EXPECT_EQ(version, 3);
        EXPECT_GT(H5Lexists(archive, "/model/sources/000000/content", H5P_DEFAULT), 0);
        EXPECT_GT(H5Lexists(archive, "/configuration/resolved/simulation/steps", H5P_DEFAULT), 0);
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
        H5Handle feature(H5Gopen2(hdf5_file, "/configuration/resolved/feature", H5P_DEFAULT), H5Gclose);
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

    const auto damaged = environment_file("custom-units-duplicate-order");
    std::filesystem::copy_file(file, damaged, std::filesystem::copy_options::overwrite_existing);
    {
        H5Handle archive(H5Fopen(damaged.string().c_str(), H5F_ACC_RDWR, H5P_DEFAULT), H5Fclose);
        H5Handle first(H5Gopen2(archive, "/model/registrations/units/0", H5P_DEFAULT), H5Gclose);
        H5Handle second(H5Gopen2(archive, "/model/registrations/units/1", H5P_DEFAULT), H5Gclose);
        H5Handle first_order(H5Aopen(first, "_DIPL_Unit_Order", H5P_DEFAULT), H5Aclose);
        H5Handle second_order(H5Aopen(second, "_DIPL_Unit_Order", H5P_DEFAULT), H5Aclose);
        uint64_t duplicate = 0;
        ASSERT_GE(H5Aread(first_order, H5T_NATIVE_UINT64, &duplicate), 0);
        ASSERT_GE(H5Awrite(second_order, H5T_NATIVE_UINT64, &duplicate), 0);
    }
    dip::Environment invalid;
    EXPECT_THROW(invalid.load(damaged), dip::IOException);
    std::filesystem::remove(damaged);

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
