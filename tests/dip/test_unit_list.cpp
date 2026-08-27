#include "pch_tests.h"

#include <filesystem>
#include <fstream>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/puq/exceptions.h>

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
    EXPECT_EQ(vnode->path.name, "foo");
    EXPECT_EQ(vnode->units_raw, unit_name);
    EXPECT_EQ(vnode->value->to_string(), "3.23");
    EXPECT_TRUE(vnode->units);
    EXPECT_EQ(vnode->units->to_string(), unit_name);
    // W/(m2*sr*m) = J/(m3*rad2*s) = kg/(m*rad2*s3) = 1e3*m-1*g*s-3*rad-2
    EXPECT_EQ(vnode->units->measurement.baseunits.dimensions().to_string(), "1e3*m-1*g*s-3*rad-2");

    // check if new unit is also in the UnitSystem::current
}

TEST(UnitList, AddUnitFromCode) {

    std::string unit_name = "Lrad"; // Radiance
    std::string unit_definition = "W/(m2*sr)";

    dip::DIP d;
    d.add_unit(unit_name, unit_definition);
    d.add_string("foo float = 3.23 " + unit_name);
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::EnvUnit& uenv = env.units.at(unit_name);
    EXPECT_EQ(uenv.name, unit_name);
    EXPECT_EQ(uenv.definition, unit_definition);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "foo");
    EXPECT_EQ(vnode->units_raw, unit_name);
    EXPECT_EQ(vnode->value->to_string(), "3.23");
    EXPECT_TRUE(vnode->units);
    EXPECT_EQ(vnode->units->to_string(), unit_name);
    // W/(m2*sr) = J/(m2*rad2*s) = kg/(rad2*s3) = 1e3*g*s-3*rad-2
    EXPECT_EQ(vnode->units->measurement.baseunits.dimensions().to_string(), "1e3*g*s-3*rad-2");
}

TEST(UnitList, UniqeNames) {

    {
        std::string unit_name = "m";
        std::string unit_definition = "W/(m2*sr)";

        try {
            dip::DIP d;
            d.add_unit(unit_name, unit_definition);
            dip::Environment env = d.parse();
            FAIL() << "Expected puq::SystemException";
        } catch (const puq::SystemException& e) {
            EXPECT_EQ(e.info().message, "Duplicate unit name");
            EXPECT_EQ(e.info().details, "A standard unit with the name `m` already exists in the current unit system.");
            EXPECT_EQ(e.info().suggestion, "Choose a different name for the custom unit.");
        } catch (...) {
            FAIL() << "Expected puq::SystemException";
        }
    }
    {
        std::string unit_name = "Lrad";
        std::string unit_definition = "W/(m2*sr)";

        try {
            dip::DIP d;
            d.add_unit(unit_name, unit_definition);
            d.add_unit(unit_name, unit_definition);
            dip::Environment env = d.parse();
            FAIL() << "Expected puq::SystemException with custom units";
        } catch (const puq::SystemException& e) {
            EXPECT_STREQ(e.what(), "Custom unit with the same name already exist in the current record: Lrad");
        } catch (const dip::EnvironmentException& e) {
            EXPECT_EQ(e.info().message, "Duplicate custom unit");
            EXPECT_EQ(e.info().details, "A custom unit named `Lrad` already exists in the environment unit list.");
            EXPECT_EQ(e.info().suggestion, "Choose a different custom unit name.");
        } catch (...) {
            FAIL() << "Expected puq::SystemException with custom units";
        }
    }
}
