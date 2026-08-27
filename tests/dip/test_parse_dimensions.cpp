#include "pch_tests.h"

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

using namespace snt;

TEST(ParseDimensions, Array2D) {

    dip::DIP d;
    d.add_string("foo int[2:,3] = [[1,1,0],[0,0,1]]");
    dip::Environment env = d.parse();

    dip::BaseNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->value_raw, val::Array::StringType({"1", "1", "0", "0", "0", "1"}));
    EXPECT_EQ(node->value_shape, val::Array::ShapeType({2, 3}));
    EXPECT_EQ(node->dtype, dip::NodeDtype::Integer);
    EXPECT_EQ(node->indent, 0);
    EXPECT_EQ(node->path.name, "foo");
    EXPECT_EQ(node->dimension, val::Array::RangeType({{2, val::Array::max_range}, {3, 3}}));
}

TEST(ParseDimensions, Array3D) {

    dip::DIP d;
    d.add_string("foo int[2:,:3,2] = [[[1,2],[3,4],[5,6]],[[7,8],[9,10],[11,12]]]");
    dip::Environment env = d.parse();

    dip::BaseNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->value_raw, val::Array::StringType({"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"}));
    EXPECT_EQ(node->value_shape, val::Array::ShapeType({2, 3, 2}));
    EXPECT_EQ(node->dtype, dip::NodeDtype::Integer);
    EXPECT_EQ(node->indent, 0);
    EXPECT_EQ(node->path.name, "foo");
    EXPECT_EQ(node->dimension, val::Array::RangeType({{2, val::Array::max_range}, {0, 3}, {2, 2}}));
}

TEST(ParseDimensions, DimensionSize) {

    dip::DIP d;
    d.add_string("foo int[2] = [[1,2],[3,4]]");
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Dimension mismatch");
        EXPECT_EQ(e.info().details, "The value has 2 dimensions, but the node has 1 dimensions.");
        EXPECT_EQ(e.info().suggestion, "Ensure that the value and node have the same number of dimensions.");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}

TEST(ParseDimensions, ExactDimensions) {

    dip::DIP d;
    d.add_string("foo int[2,3] = [[1,2,3],[4,5,6]]");
    dip::Environment env = d.parse();
    dip::BaseNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->value_raw, val::Array::StringType({"1", "2", "3", "4", "5", "6"}));
    EXPECT_EQ(node->value_shape, val::Array::ShapeType({2, 3}));

    d = dip::DIP();
    d.add_string("foo int[2,3] = [[1,2,3,4]]");
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Dimension range mismatch");
        EXPECT_EQ(
            e.info().details, "The value dimensions `[1,4]` do not correspond to the node dimension ranges `[2,3]`."
        );
        EXPECT_EQ(
            e.info().suggestion,
            "Ensure that each value dimension falls within the corresponding dimension range of the node."
        );
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}

TEST(ParseDimensions, DimensionRanges) {

    dip::DIP d;
    d.add_string("foo int[:2,2:] = [[1,2,3]]");
    dip::Environment env = d.parse();
    dip::BaseNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->value_raw, val::Array::StringType({"1", "2", "3"}));
    EXPECT_EQ(node->value_shape, val::Array::ShapeType({1, 3}));

    d = dip::DIP();
    d.add_string("foo int[2:,:2] = [[1,2,3]]");
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Dimension range mismatch");
        EXPECT_EQ(
            e.info().details, "The value dimensions `[1,3]` do not correspond to the node dimension ranges `[2:,:2]`."
        );
        EXPECT_EQ(
            e.info().suggestion,
            "Ensure that each value dimension falls within the corresponding dimension range of the node."
        );
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}
