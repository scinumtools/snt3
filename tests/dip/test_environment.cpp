#include "pch_tests.h"

#include <filesystem>
#include <hdf5.h>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>

using namespace snt;

namespace {
    class H5Handle {
      public:
        using Closer = herr_t (*)(hid_t);

        H5Handle(hid_t id, Closer closer) : id_(id), closer_(closer) {}
        H5Handle(const H5Handle&) = delete;
        H5Handle& operator=(const H5Handle&) = delete;
        ~H5Handle() {
            if (id_ >= 0)
                closer_(id_);
        }
        operator hid_t() const { return id_; }

      private:
        hid_t id_;
        Closer closer_;
    };

    std::string hdf5_string_attribute(hid_t object, const char* name) {
        H5Handle attribute(H5Aopen(object, name, H5P_DEFAULT), H5Aclose);
        H5Handle type(H5Aget_type(attribute), H5Tclose);
        const size_t size = H5Tget_size(type);
        std::vector<char> value(size + 1, '\0');
        EXPECT_GE(H5Aread(attribute, type, value.data()), 0);
        return value.data();
    }

    void expect_hdf5_string_attribute(hid_t object, const char* name, const std::string& expected) {
        ASSERT_GT(H5Aexists(object, name), 0);
        EXPECT_EQ(hdf5_string_attribute(object, name), expected);
    }

    uint8_t hdf5_uint8_attribute(hid_t object, const char* name) {
        H5Handle attribute(H5Aopen(object, name, H5P_DEFAULT), H5Aclose);
        uint8_t value = 0;
        EXPECT_GE(H5Aread(attribute, H5T_NATIVE_UINT8, &value), 0);
        return value;
    }

    dip::Environment parsed_environment() {
        dip::DIP parser;
        parser.add_string(
            "title str = \"C++ environment\"\n"
            "  !options [\"C++ environment\", \"Other\"]\n"
            "  !tags [\"example\", \"hdf5\"]\n"
            "  ?descr \"Environment round-trip fixture\"\n"
            "simulation\n"
            "  steps int = 100\n"
            "  timestep float = 0.5 fs\n"
            "  enabled bool = true\n"
            "  restart_file str = none\n"
            "boundary[inlet]\n"
            "  velocity float[3] = [1.0, 0.0, 0.0] m/s\n"
            "samples[]\n"
            "  time float = 0 s\n"
            "samples[]\n"
            "  time float = 1 s"
        );
        return parser.parse();
    }

    std::filesystem::path environment_file(const std::string& name) {
        return std::filesystem::temp_directory_path() / ("scinumtools3-" + name + ".diph5");
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
    EXPECT_EQ(env["boundary"].items().size(), 1);
    EXPECT_EQ(env["samples"].elements().size(), 2);
    EXPECT_EQ(env["samples[1].time"].as<double>(), 1.0);
    const auto velocity = env["boundary[inlet].velocity"].as<std::array<double, 3>>();
    EXPECT_EQ(velocity, (std::array<double, 3>{1.0, 0.0, 0.0}));
    EXPECT_EQ(env.get_node("title")->tags, (val::Array::StringType{"example", "hdf5"}));
    ASSERT_EQ(env.get_node("title")->options.size(), 2);
    EXPECT_EQ(env.get_node("title")->options[1].value_raw, "Other");
    EXPECT_EQ(env.get_node("title")->metadata.description, "Environment round-trip fixture");

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
    dip::Environment env = parsed_environment();
    ASSERT_EQ(env.nodes.size(), 8);

    env.save(file);
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
        expect_hdf5_string_attribute(hdf5_file, "_DIPL_Format", "SciNumTools3 Environment");

        H5Handle version_attribute(H5Aopen(hdf5_file, "_DIPL_Schema_Version", H5P_DEFAULT), H5Aclose);
        uint64_t version = 0;
        ASSERT_GE(H5Aread(version_attribute, H5T_NATIVE_UINT64, &version), 0);
        EXPECT_EQ(version, 1);

        ASSERT_GT(H5Lexists(hdf5_file, "/simulation", H5P_DEFAULT), 0);
        H5Handle simulation(H5Gopen2(hdf5_file, "/simulation", H5P_DEFAULT), H5Gclose);
        expect_hdf5_string_attribute(simulation, "_DIPL_Kind", "group");
        expect_hdf5_string_attribute(simulation, "_DIPL_Path", "simulation");

    H5Handle steps(H5Dopen2(hdf5_file, "/simulation/steps", H5P_DEFAULT), H5Dclose);
    H5Handle steps_space(H5Dget_space(steps), H5Sclose);
    EXPECT_EQ(H5Sget_simple_extent_ndims(steps_space), 0);
    H5Handle steps_type(H5Dget_type(steps), H5Tclose);
    EXPECT_EQ(H5Tget_class(steps_type), H5T_INTEGER);
    EXPECT_EQ(H5Tget_size(steps_type), sizeof(int32_t));
    expect_hdf5_string_attribute(steps, "_DIPL_Path", "simulation.steps");
    expect_hdf5_string_attribute(steps, "_DIPL_Value_Type", "int32");

    H5Handle boundary(H5Gopen2(hdf5_file, "/boundary", H5P_DEFAULT), H5Gclose);
    expect_hdf5_string_attribute(boundary, "_DIPL_Kind", "map");
    expect_hdf5_string_attribute(boundary, "_DIPL_Path", "boundary");
    H5Handle inlet(H5Gopen2(hdf5_file, "/boundary/inlet", H5P_DEFAULT), H5Gclose);
    expect_hdf5_string_attribute(inlet, "_DIPL_Kind", "map_item");
    expect_hdf5_string_attribute(inlet, "_DIPL_Key", "inlet");

    H5Handle velocity(H5Dopen2(hdf5_file, "/boundary/inlet/velocity", H5P_DEFAULT), H5Dclose);
    H5Handle velocity_space(H5Dget_space(velocity), H5Sclose);
    hsize_t velocity_size = 0;
    ASSERT_EQ(H5Sget_simple_extent_ndims(velocity_space), 1);
    ASSERT_EQ(H5Sget_simple_extent_dims(velocity_space, &velocity_size, nullptr), 1);
    EXPECT_EQ(velocity_size, 3);
    H5Handle velocity_type(H5Dget_type(velocity), H5Tclose);
    EXPECT_EQ(H5Tget_class(velocity_type), H5T_FLOAT);
    EXPECT_EQ(H5Tget_size(velocity_type), sizeof(double));
    EXPECT_EQ(hdf5_uint8_attribute(velocity, "_DIPL_Array"), 1);
    expect_hdf5_string_attribute(velocity, "units", "m*s-1");

    H5Handle samples(H5Gopen2(hdf5_file, "/samples", H5P_DEFAULT), H5Gclose);
    expect_hdf5_string_attribute(samples, "_DIPL_Kind", "list");
    H5Handle sample_zero(H5Gopen2(hdf5_file, "/samples/0", H5P_DEFAULT), H5Gclose);
    expect_hdf5_string_attribute(sample_zero, "_DIPL_Kind", "list_item");
    H5Handle sample_one(H5Gopen2(hdf5_file, "/samples/1", H5P_DEFAULT), H5Gclose);
    expect_hdf5_string_attribute(sample_one, "_DIPL_Kind", "list_item");

    H5Handle title(H5Dopen2(hdf5_file, "/title", H5P_DEFAULT), H5Dclose);
    expect_hdf5_string_attribute(title, "description", "Environment round-trip fixture");
    EXPECT_GT(H5Aexists(title, "_DIPL_Tags"), 0);
    EXPECT_GT(H5Aexists(title, "_DIPL_Options"), 0);

    }

    std::filesystem::remove(file);
}

TEST(Environment, Generate) {
    dip::Environment env = parsed_environment();
    ASSERT_EQ(env.nodes.size(), 8);

    EXPECT_THROW(env.generate(dip::ExportFormat::JSON, "parameters.json"), dip::MissingException);
}

TEST(Environment, RequestValue) {

    dip::DIP d;
    d.add_string("foo bool = true");
    d.add_string("bar int = 3 J");
    dip::Environment env = d.parse();

    // request value as is
    val::BaseValue::PointerType node = env.request_value("?foo");
    EXPECT_EQ(node->to_string(), "true");

    // request value with a specific unit
    node = env.request_value("?bar", dip::RequestType::Reference, "erg");
    EXPECT_EQ(node->to_string(), "3e7");
}

TEST(Environment, RequestGroup) {

    dip::DIP d;
    d.add_string("foo.bar bool = true");
    d.add_string("  !tags [\"snap\",\"crackle\"]");
    d.add_string("foo.baz int = 3");
    d.add_string("  !tags [\"crackle\",\"pop\"]");
    dip::Environment env = d.parse();

    // select all children
    dip::ValueNode::ListType nodes = env.request_group("?foo.");
    EXPECT_EQ(nodes.at(0)->to_string(), "true");
    EXPECT_EQ(nodes.at(1)->to_string(), "3");

    // filter selection using tags
    nodes = env.request_group("?foo.", dip::RequestType::Reference, {"crackle"});
    EXPECT_EQ(nodes.size(), 2);
    EXPECT_EQ(nodes.at(0)->path.name, "bar");
    EXPECT_EQ(nodes.at(1)->path.name, "baz");

    nodes = env.request_group("?foo.", dip::RequestType::Reference, {"snap"});
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes.at(0)->path.name, "bar");

    nodes = env.request_group("?foo.", dip::RequestType::Reference, {"pop"});
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes.at(0)->path.name, "baz");
}

TEST(Environment, RequestMap) {

    dip::DIP d;
    d.add_string("jerk.snap[crackle]");
    d.add_string("  foo int = 3");
    d.add_string("jerk.snap[pop]");
    d.add_string("  bar int = 4");
    dip::Environment env = d.parse();

    // select all items
    std::unordered_map<std::string, dip::ValueNode::ListType> map = env.request_map("?jerk.snap");
    EXPECT_EQ(map.size(), 2);
    {
        auto it = map.find("crackle");
        EXPECT_NE(it, map.end());
        dip::ValueNode::ListType nodes = it->second;
        EXPECT_EQ(nodes.at(0)->path.name, "foo");
    }
    {
        auto it = map.find("pop");
        EXPECT_NE(it, map.end());
        dip::ValueNode::ListType nodes = it->second;
        EXPECT_EQ(nodes.at(0)->path.name, "bar");
    }
}

TEST(Environment, RequestList) {

    dip::DIP d;
    d.add_string("jerk.snap[]");
    d.add_string("  foo int = 3");
    d.add_string("jerk.snap[]");
    d.add_string("  bar int = 4");
    dip::Environment env = d.parse();

    // select all items
    std::vector<dip::ValueNode::ListType> list = env.request_list("?jerk.snap");
    EXPECT_EQ(list.size(), 2);
    {
        const dip::ValueNode::ListType& nodes = list.at(0);
        EXPECT_EQ(nodes.at(0)->path.name, "foo");
    }
    {
        const dip::ValueNode::ListType& nodes = list.at(1);
        EXPECT_EQ(nodes.at(0)->path.name, "bar");
    }
}

TEST(Environment, RequestNodeData) {

    dip::DIP d;
    d.add_string("foo.bar bool = true");
    d.add_string("foo.baz float = 3.45 J");
    dip::Environment env = d.parse();
    dip::ValueNodeData data = env.request_node_data("?foo.bar");
    EXPECT_TRUE(data.value);
    if (data.value) {
        EXPECT_EQ(data.value->to_string(), "true");
    }

    data = env.request_node_data("?foo.baz");
    EXPECT_TRUE(data.value && data.units);
    if (data.value) {
        EXPECT_EQ(data.value->to_string(), "3.45");
        EXPECT_EQ(data.units->to_string(), "J");
    }
}

TEST(Environment, RequestCursor) {

    dip::DIP d;
    d.add_string(
        "foo\n"
        "  bar bool = true\n"
        "  baz int = 3\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);

    {
        bool scalar = env["foo.bar"].as<bool>();
        EXPECT_EQ(scalar, true);
    }
    {
        int64_t scalar = env["foo.baz"].as<int64_t>();
        EXPECT_EQ(scalar, 3);
    }
}

TEST(Environment, GetNode) {

    dip::DIP d;
    d.add_string(
        "foo\n"
        "  baz int = 3 cm\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::ValueNode::PointerType node = env.get_node("foo.baz");
    EXPECT_EQ(node->path.name, "foo.baz");
    EXPECT_EQ(node->value->to_string(), "3");
    EXPECT_EQ(node->units->to_string(), "cm");
}

// TODO: tests more requests with RequestType::Function
