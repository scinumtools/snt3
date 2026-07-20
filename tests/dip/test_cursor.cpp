#include "pch_tests.h"

#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

using namespace snt;

class Cursor : public ::testing::Test {
  protected:
    dip::DIP d;
    dip::Environment env;

    void SetUp() override {
        d.add_string(
            "foo[bar]\n"
            "  jerk bool = true\n"
            "  snap int = 3\n"
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

TEST_F(Cursor, TraverseCollections2) {

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

    // traverse parameter tree and save collection paths and values
    std::vector<std::string> params_parsed;
    params_parsed.reserve(10);
    dip::Cursor group = env["jerk.baz"];
    params_parsed.push_back(group.to_string());
    const std::unordered_map<std::string, dip::Cursor> map = env["jerk.snap"].items();
    for (auto [key, item] : map) {
        params_parsed.push_back(item.to_string());
        dip::Cursor group = item["bar"];
        if (key == "crackle") {
            const std::vector<dip::Cursor> list = item["pop"].elements();
            for (auto item : list) {
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
}

TEST_F(Cursor, ValuesFullPath) {

    EXPECT_EQ(env.nodes.size(), 8);

    {
        bool jerk = env["foo[bar].jerk"].as<bool>();
        int64_t snap = env["foo[bar].snap"].as<int64_t>();
        double crackle = env["foo[baz].yippee.crackle"].as<double>();
        std::string pop = env["foo[baz].yippee.pop"].as<std::string>();
    }
    {
        std::array<bool, 3> jerk = env["foo[baz].qux[0].jerk"].as<std::array<bool, 3>>();
        std::array<int64_t, 3> snap = env["foo[baz].qux[0].snap"].as<std::array<int64_t, 3>>();
        std::array<double, 2> crackle = env["foo[baz].qux[1].crackle"].as<std::array<double, 2>>();
        std::array<std::string, 2> pop = env["foo[baz].qux[1].pop"].as<std::array<std::string, 2>>();
    }
    {
        std::vector<bool> jerk = env["foo[baz].qux[0].jerk"].as<std::vector<bool>>();
        std::vector<int64_t> snap = env["foo[baz].qux[0].snap"].as<std::vector<int64_t>>();
        std::vector<double> crackle = env["foo[baz].qux[1].crackle"].as<std::vector<double>>();
        std::vector<std::string> pop = env["foo[baz].qux[1].pop"].as<std::vector<std::string>>();
    }
}

TEST_F(Cursor, ValuesPartialPaths) {

    EXPECT_EQ(env.nodes.size(), 8);

    {
        bool jerk = env["foo"]["bar"]["jerk"].as<bool>();
        int64_t snap = env["foo[bar]"]["snap"].as<int64_t>();
        double crackle = env["foo"]["baz"]["yippee"]["crackle"].as<double>();
        std::string pop = env["foo[baz]"]["yippee.pop"].as<std::string>();
    }
    {
        std::array<bool, 3> jerk = env["foo"]["baz"]["qux"][0]["jerk"].as<std::array<bool, 3>>();
        std::array<int64_t, 3> snap = env["foo[baz]"]["qux[0]"]["snap"].as<std::array<int64_t, 3>>();
        std::array<double, 2> crackle = env["foo"]["baz"]["qux"][1]["crackle"].as<std::array<double, 2>>();
        std::array<std::string, 2> pop = env["foo[baz]"]["qux[1]"]["pop"].as<std::array<std::string, 2>>();
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
