#include "pch_tests.h"

#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>

using namespace snt;

class Cursor : public ::testing::Test {
  protected:
    dip::DIP d;
    dip::Environment env;

    void SetUp() override {
        d.add_string(
            "foo[bar]\n"
            "  jerk bool = true\n"
            "  snap int = 3 cm\n"
            "foo[baz]\n"
            "  yippee\n"
            "    crackle float = 4e5\n"
            "    pop str = \"shot\"\n"
            "  qux[]\n"
            "    jerk bool[3] = [true, false, true]\n"
            "    snap int[3] = [3,2,1]\n"
            "  qux[]\n"
            "    crackle float[2] = [4e5, 34e2]\n"
            "    pop str[2] = [\"shot\",\"puff\"]\n"
        );
        env = d.parse();
    }
};

TEST_F(Cursor, TraverseCollections) {

    dip::DIP d;
    d.add_string(
        "jerk.snap[crackle]\n"
        "  bar str = \"pot\"\n"
        "  pop[]\n"
        "    foo bool = true\n"
        "  pop[]\n"
        "    foo int = 3\n"
        "  pop[]\n"
        "    foo float = 4e5\n"
        "jerk.snap[lock]\n"
        "  bar str = \"shot\"\n"
        "jerk.baz bool = false\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);

    // test path kinds
    EXPECT_EQ(env["jerk.baz"].get_kind(), dip::Path::Kind::Group);
    EXPECT_EQ(env["jerk.snap"].get_kind(), dip::Path::Kind::Map);
    EXPECT_EQ(env["jerk.snap[crackle].pop"].get_kind(), dip::Path::Kind::List);

    // traverse parameter tree and save collection paths and values
    std::vector<std::string> params_parsed;
    params_parsed.reserve(10);
    dip::Cursor group = env["jerk.baz"];
    params_parsed.push_back(group.to_string());
    const std::unordered_map<std::string, dip::Cursor> map = env["jerk.snap"].items();
    for (const auto& [key, item] : map) {
        params_parsed.push_back(item.to_string());
        dip::Cursor group = item["bar"];
        if (key == "crackle") {
            const std::vector<dip::Cursor> list = item["pop"].elements();
            for (const auto& item : list) {
                params_parsed.push_back(item.to_string());
                dip::Cursor group = item["foo"];
                params_parsed.push_back(group.to_string());
            }
        } else if (key == "lock") {
            dip::Cursor group = item["bar"];
            params_parsed.push_back(group.to_string());
        }
    }
    std::sort(params_parsed.begin(), params_parsed.end());

    // prepare reference parameter set
    std::vector<std::string> params_ref = {
        "Cursor('jerk.baz', false)",
        "Cursor('jerk.snap[crackle]')",
        "Cursor('jerk.snap[crackle].pop[0]')",
        "Cursor('jerk.snap[crackle].pop[0].foo', true)",
        "Cursor('jerk.snap[crackle].pop[1]')",
        "Cursor('jerk.snap[crackle].pop[1].foo', 3)",
        "Cursor('jerk.snap[crackle].pop[2]')",
        "Cursor('jerk.snap[crackle].pop[2].foo', 4e5)",
        "Cursor('jerk.snap[lock]')",
        "Cursor('jerk.snap[lock].bar', \"shot\")",
    };

    // compare the two parameter sets
    EXPECT_EQ(params_parsed, params_ref);

    // test if map collection has an item
    EXPECT_TRUE(env["jerk.snap"].has_item("crackle"));
    EXPECT_FALSE(env["jerk.snap"].has_item("flock"));

    // test to find an item other than a map collection
    try {
        env["jerk.snap[crackle].pop"].has_item("flop");
        FAIL() << "Expected dip::EnvironmentException";
    } catch (const dip::EnvironmentException& e) {
        EXPECT_EQ(e.info().message, "Wrong collection kind");
        EXPECT_EQ(
            e.info().details,
            "The path `jerk.snap[crackle].pop` must correspond to a map collection, but it refers to a list collection."
        );
        EXPECT_EQ(e.info().suggestion, "Check whether the path is correct.");
    } catch (...) {
        FAIL() << "Expected dip::EnvironmentException";
    }
}

TEST_F(Cursor, ValuesFullPath) {

    EXPECT_EQ(env.nodes.size(), 8);

    {
        const bool jerk = env["foo[bar].jerk"].as<bool>();
        const int64_t snap = env["foo[bar].snap"].as<int64_t>();
        const double crackle = env["foo[baz].yippee.crackle"].as<double>();
        std::string pop = env["foo[baz].yippee.pop"].as<std::string>();
        EXPECT_TRUE(jerk);
        EXPECT_EQ(snap, 3);
        EXPECT_EQ(crackle, 4e5);
        EXPECT_EQ(pop, "shot");
    }
    {
        const auto jerk = env["foo[baz].qux[0].jerk"].as<std::array<bool, 3>>();
        const auto snap = env["foo[baz].qux[0].snap"].as<std::array<int64_t, 3>>();
        const auto crackle = env["foo[baz].qux[1].crackle"].as<std::array<double, 2>>();
        std::array<std::string, 2> pop = env["foo[baz].qux[1].pop"].as<std::array<std::string, 2>>();
        EXPECT_EQ(jerk, (std::array<bool, 3>{true, false, true}));
        EXPECT_EQ(snap, (std::array<int64_t, 3>{3, 2, 1}));
        EXPECT_EQ(crackle, (std::array<double, 2>{4e5, 34e2}));
        EXPECT_EQ(pop, (std::array<std::string, 2>{"shot", "puff"}));
    }
    {
        std::vector<bool> jerk = env["foo[baz].qux[0].jerk"].as<std::vector<bool>>();
        std::vector<int64_t> snap = env["foo[baz].qux[0].snap"].as<std::vector<int64_t>>();
        std::vector<double> crackle = env["foo[baz].qux[1].crackle"].as<std::vector<double>>();
        std::vector<std::string> pop = env["foo[baz].qux[1].pop"].as<std::vector<std::string>>();
    }
}

TEST_F(Cursor, LowerPrecisionNumericValues) {
    dip::DIP input;
    input.add_string(
        "signed int = 127\n"
        "unsigned uint = 255\n"
        "real float = 1.25\n"
        "signed_values int[2] = [3, 4]\n"
        "unsigned_values uint[2] = [5, 6]\n"
        "real_values float[2] = [1.25, 2.5]\n"
    );
    const dip::Environment lower = input.parse();

    EXPECT_EQ(lower["signed"].as<int8_t>(), 127);
    EXPECT_EQ(lower["unsigned"].as<uint8_t>(), 255);
    EXPECT_FLOAT_EQ(lower["real"].as<float>(), 1.25F);
    EXPECT_EQ(lower["signed_values"].as<std::vector<int16_t>>(), (std::vector<int16_t>{3, 4}));
    EXPECT_EQ((lower["unsigned_values"].as<std::array<uint32_t, 2>>()), (std::array<uint32_t, 2>{5, 6}));
    EXPECT_EQ((lower["real_values"].as<std::array<float, 2>>()), (std::array<float, 2>{1.25F, 2.5F}));

    // Cursor conversions are views; narrowing does not alter DIPL's internal storage.
    EXPECT_EQ(lower["signed"].as<int64_t>(), 127);
    EXPECT_DOUBLE_EQ(lower["real"].as<double>(), 1.25);

    // Numeric categories remain distinct: integer values are not floating-point values.
    EXPECT_THROW(lower["signed"].as<float>(), dip::SyntaxException);
}

TEST_F(Cursor, ValuesPartialPaths) {

    EXPECT_EQ(env.nodes.size(), 8);

    {
        const bool jerk = env["foo"]["bar"]["jerk"].as<bool>();
        const int64_t snap = env["foo[bar]"]["snap"].as<int64_t>();
        const double crackle = env["foo"]["baz"]["yippee"]["crackle"].as<double>();
        std::string pop = env["foo[baz]"]["yippee.pop"].as<std::string>();
        EXPECT_TRUE(jerk);
        EXPECT_EQ(snap, 3);
        EXPECT_EQ(crackle, 4e5);
        EXPECT_EQ(pop, "shot");
    }
    {
        const auto jerk = env["foo"]["baz"]["qux"][0]["jerk"].as<std::array<bool, 3>>();
        const auto snap = env["foo[baz]"]["qux[0]"]["snap"].as<std::array<int64_t, 3>>();
        const auto crackle = env["foo"]["baz"]["qux"][1]["crackle"].as<std::array<double, 2>>();
        std::array<std::string, 2> pop = env["foo[baz]"]["qux[1]"]["pop"].as<std::array<std::string, 2>>();
        EXPECT_EQ(jerk, (std::array<bool, 3>{true, false, true}));
        EXPECT_EQ(snap, (std::array<int64_t, 3>{3, 2, 1}));
        EXPECT_EQ(crackle, (std::array<double, 2>{4e5, 34e2}));
        EXPECT_EQ(pop, (std::array<std::string, 2>{"shot", "puff"}));
    }
}

TEST_F(Cursor, ParseCollections) {

    EXPECT_EQ(env.nodes.size(), 8);

    const std::unordered_map<std::string, dip::Collection> colls = env.hierarchy.get_collections();
    std::vector<std::string> colls_parsed;
    colls_parsed.reserve(colls.size());
    for (const auto& [name, coll] : colls)
        colls_parsed.push_back(name);
    std::sort(colls_parsed.begin(), colls_parsed.end());

    // prepare reference parameter set
    std::vector<std::string> colls_ref = {
        "foo",
        "foo[bar]",
        "foo[bar].jerk",
        "foo[bar].snap",
        "foo[baz]",
        "foo[baz].qux",
        "foo[baz].qux[0]",
        "foo[baz].qux[0].jerk",
        "foo[baz].qux[0].snap",
        "foo[baz].qux[1]",
        "foo[baz].qux[1].crackle",
        "foo[baz].qux[1].pop",
        "foo[baz].yippee",
        "foo[baz].yippee.crackle",
        "foo[baz].yippee.pop"
    };

    // compare parsed and expected collection names
    EXPECT_EQ(colls_parsed, colls_ref);
}

TEST_F(Cursor, GetValueNode) {
    {
        dip::ValueNode::PointerType node = env["foo[bar].snap"].get_node();
        EXPECT_EQ(node->path.name, "foo[bar].snap");
        EXPECT_EQ(node->value->to_string(), "3");
        EXPECT_EQ(node->units->to_string(), "cm");
    }
    {
        val::BaseValue::PointerType value = env["foo[bar].snap"].get_value();
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(value->to_string(), "3");
    }
    {
        std::optional<puq::Quantity> units = env["foo[bar].snap"].get_units();
        EXPECT_TRUE(units.has_value());
        EXPECT_EQ(units->to_string(), "cm");
    }
}
