#include "pch_tests.h"

#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

using namespace snt;

TEST(Cursor, TraverseCollections) {

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
    dip::Cursor cursor = env.request_cursor();

    // traverse parameter tree and save collection paths and values
    std::vector<std::string> params_parsed;
    params_parsed.reserve(10);
    dip::Cursor group = cursor.get_group("jerk.baz");
    params_parsed.push_back(group.to_string());
    const std::unordered_map<std::string, dip::Cursor> map = cursor.get_map("jerk.snap");
    for (auto [key, item] : map) {
        params_parsed.push_back(item.get_path());
        dip::Cursor group = item.get_group("bar");
        if (key == "crackle") {
            const std::vector<dip::Cursor> list = item.get_list("pop");
            for (auto item : list) {
                params_parsed.push_back(item.get_path());
                dip::Cursor group = item.get_group("foo");
                params_parsed.push_back(group.to_string());
            }
        } else if (key == "lock") {
            dip::Cursor group = item.get_group("bar");
            params_parsed.push_back(group.to_string());
        }
    }
    std::sort(params_parsed.begin(), params_parsed.end());

    // prepare reference parameter set
    std::vector<std::string> params_ref = {
        "jerk.baz = false",
        "jerk.snap[crackle]",
        "jerk.snap[crackle].pop[0]",
        "jerk.snap[crackle].pop[0].foo = true",
        "jerk.snap[crackle].pop[1]",
        "jerk.snap[crackle].pop[1].foo = 3",
        "jerk.snap[crackle].pop[2]",
        "jerk.snap[crackle].pop[2].foo = 4e5",
        "jerk.snap[lock]",
        "jerk.snap[lock].bar = \"shot\"",
    };

    // compare the two parameter sets
    EXPECT_EQ(params_parsed, params_ref);
}

TEST(Cursor, RetrieveValues) {

    dip::DIP d;
    d.add_string(
        "jerk.snap[crackle]\n"
        "  bar str = \"pot\"\n"
        "  pop[]\n"
        "    foo bool[3] = [true, false, true]\n"
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
    dip::Cursor cursor = env.request_cursor();

    // test values directly from groups
    {
        dip::Cursor group = cursor.get_group("jerk.baz");
        std::vector<uint8_t> vector = group.to_vector<uint8_t>();
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(vector[0], false);
        uint8_t scalar = group.to_scalar<uint8_t>();
        EXPECT_EQ(scalar, false);
    }
    {
        dip::Cursor group = cursor.get_group("jerk.snap[crackle].pop[0].foo");
        EXPECT_EQ(group.get_shape(), std::vector<size_t>({3}));
        std::vector<uint8_t> vector = group.to_vector<uint8_t>();
        EXPECT_EQ(vector.size(), 3);
        EXPECT_EQ(vector, std::vector<uint8_t>({true, false, true}));
        int64_t scalar = group.to_scalar<uint8_t>();
        EXPECT_EQ(scalar, true);
    }
    {
        dip::Cursor group = cursor.get_group("jerk.snap[crackle].pop[1].foo");
        std::vector<int64_t> vector = group.to_vector<int64_t>();
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(vector[0], 3);
        int64_t scalar = group.to_scalar<int64_t>();
        EXPECT_EQ(scalar, 3);
    }
    {
        dip::Cursor group = cursor.get_group("jerk.snap[crackle].pop[2].foo");
        std::vector<double> vector = group.to_vector<double>();
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(vector[0], 4e5);
        double scalar = group.to_scalar<double>();
        EXPECT_EQ(scalar, 4e5);
    }
    {
        dip::Cursor group = cursor.get_group("jerk.snap[lock].bar");
        std::vector<std::string> vector = group.to_vector<std::string>();
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(vector[0], "shot");
        std::string scalar = group.to_scalar<std::string>();
        EXPECT_EQ(scalar, "shot");
    }
}
