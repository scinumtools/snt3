#include "pch_tests.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <snt/dip/comparison.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

using namespace snt;

namespace {
    dip::Environment parsed(const std::string& code) {
        dip::DIP parser;
        parser.add_string(code);
        return parser.parse();
    }
} // namespace

TEST(Comparison, ValuesAndBoundedText) {
    auto before = parsed("keep int = 1\nremove int = 2\narray int[4] = [1,2,3,4]\n");
    auto after = parsed("keep int = 1\nadd int = 3\narray int[4] = [1,9,3,8]\n");
    const auto result = dip::compare(before, after);
    EXPECT_FALSE(result.equal());
    EXPECT_EQ(result.added, 1);
    EXPECT_EQ(result.removed, 1);
    EXPECT_EQ(result.changed, 1);
    ASSERT_EQ(result.differences.size(), 3);
    EXPECT_EQ(result.differences[1].path, "array");
    EXPECT_EQ(result.differences[1].changed_elements, 2);
    EXPECT_EQ(result.differences[1].example_indices, (std::vector<std::size_t>{1, 3}));
    const auto text = dip::render_comparison(result, 1);
    EXPECT_NE(text.find("1 added, 1 removed, 1 changed"), std::string::npos);
    EXPECT_NE(text.find("... 2 more differences"), std::string::npos);
    const dip::Comparison comparison{before, after};
    EXPECT_EQ(comparison.result().differences.size(), 3);
    EXPECT_EQ(comparison.render(1), text);
}

TEST(Comparison, ScopeAndDiph5RoundTrip) {
    auto before = parsed("value int = 1\n  ?descr \"old\"\n");
    auto after = parsed("value int = 1\n  ?descr \"new\"\n");
    EXPECT_TRUE(dip::compare(before, after).equal());
    dip::ComparisonOptions options;
    options.scope = dip::ComparisonScope::Full;
    const auto full = dip::compare(before, after, options);
    EXPECT_FALSE(full.equal());
    const auto value =
        std::find_if(full.differences.begin(), full.differences.end(), [](const dip::Difference& difference) {
            return difference.category == "value";
        });
    ASSERT_NE(value, full.differences.end());
    EXPECT_NE(std::find(value->fields.begin(), value->fields.end(), "metadata"), value->fields.end());

    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto first = std::filesystem::temp_directory_path() / ("snt-compare-before-" + suffix + ".diph5");
    const auto second = std::filesystem::temp_directory_path() / ("snt-compare-after-" + suffix + ".diph5");
    before.save(first);
    after.save(second);
    EXPECT_TRUE(dip::compare_diph5(first, second).equal());
    EXPECT_FALSE(dip::compare_diph5(first, second, options).equal());
    const dip::Comparison comparison{first, second, options};
    std::filesystem::remove(first);
    std::filesystem::remove(second);
    EXPECT_FALSE(comparison.equal());
    EXPECT_EQ(comparison.result().scope, dip::ComparisonScope::Full);
    EXPECT_NE(comparison.render().find("Full DIPH5 comparison"), std::string::npos);
}
