#include "pch_tests.h"

#include <filesystem>
#include <fstream>
#include <snt/dip/dip.h>

using namespace snt;

TEST(UnitList, Keyword) {

    std::string unit_name = "B_lam"; // Spectral Radiance
    std::string unit_definition = "W/(m2*sr*m)";

    dip::DIP d;
    d.add_string("$unit " + unit_name + " = " + unit_definition);
    d.add_string("foo float = 3.23 " + unit_name);
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    // check if new unit is in the unit list
    dip::EnvUnit& uenv = env.units.at(unit_name);
    EXPECT_EQ(uenv.name, unit_name);
    EXPECT_EQ(uenv.definition, unit_definition);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->name, "foo");
    EXPECT_EQ(vnode->units_raw, unit_name);
    EXPECT_EQ(vnode->value->to_string(), "3.23");
    EXPECT_TRUE(vnode->units);
    EXPECT_EQ(vnode->units->to_string(), unit_name);
    // W/(m2*sr*m) = J/(m3*rad2*s) = kg/(m*rad2*s3) = 1e3*m-1*g*s-3*rad-2
    EXPECT_EQ(vnode->units->measurement.baseunits.dimensions().to_string(), "1e3*m-1*g*s-3*rad-2");

    // check if new unit is also in the UnitSystem::current
}

TEST(UnitList, AddUnitFromCode) {

    std::string unit_name = "B_lam"; // Spectral Radiance
    std::string unit_definition = "W/(m2*sr*m)";

    dip::DIP d;
    d.add_unit(unit_name, unit_definition);
    d.add_string("foo float = 3.23 " + unit_name);
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::EnvUnit& uenv = env.units.at(unit_name);
    EXPECT_EQ(uenv.name, unit_name);
    EXPECT_EQ(uenv.definition, unit_definition);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->name, "foo");
    EXPECT_EQ(vnode->units_raw, unit_name);
    EXPECT_EQ(vnode->value->to_string(), "3.23");
    EXPECT_TRUE(vnode->units);
    EXPECT_EQ(vnode->units->to_string(), unit_name);
    // W/(m2*sr*m) = J/(m3*rad2*s) = kg/(m*rad2*s3) = 1e3*m-1*g*s-3*rad-2
    EXPECT_EQ(vnode->units->measurement.baseunits.dimensions().to_string(), "1e3*m-1*g*s-3*rad-2");
}
