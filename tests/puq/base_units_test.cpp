#include "pch_tests.h"

#include <snt/puq/base_units.h>
#include <snt/puq/math/pow.h>

using namespace snt;

TEST(BaseUnits, Initialization) {

  // Appending BaseUnit-s
  puq::BaseUnits bus;
  puq::BaseUnit bu1 = {"k", "g", 2};
  bus.append(bu1);
  puq::BaseUnit bu2("m", "s", 3);
  bus.append(bu2);
  bus.append("c", "m", 1);
  EXPECT_EQ(bus.size(), 3);
  EXPECT_EQ(bus.to_string(), "kg2*ms3*cm");

  // Appending same unit with a different prefix
  bus = puq::BaseUnits();
  bus.append("c", "m", 1);
  bus.append("k", "m", 2);
  EXPECT_EQ(bus.to_string(), "cm*km2");

  // Using a string expression
  bus = puq::BaseUnits("kg*m2/s2");
  EXPECT_EQ(bus.to_string(), "kg*m2*s-2");

  bus = puq::BaseUnits("km/s");
  std::stringstream ss;
  ss << bus; // cast base units as a string into a stream
  EXPECT_EQ(ss.str(), "km*s-1");
}

TEST(BaseUnits, InitializationFractions) {

  puq::BaseUnits bus;
  puq::BaseUnit bu1 = {"k", "g", 2, 1};
  bus.append(bu1);
  puq::BaseUnit bu2("m", "s", 3, 2);
  bus.append(bu2);
  bus.append("c", "m", 1, 1);
  EXPECT_EQ(bus.size(), 3);
  EXPECT_EQ(bus.to_string(), "kg2*ms3:2*cm");
}

TEST(BaseUnits, RemoveZeroExponents) {

  puq::BaseUnits bus;
  bus.append("m", "s", 2); // adding normal units
  bus.append("c", "m", 1);
  EXPECT_EQ(bus.size(), 2);
  EXPECT_EQ(bus.to_string(), "ms2*cm");

  bus.append("c", "m", -1); // adding inverse unit
  EXPECT_EQ(bus.size(), 1);
  EXPECT_EQ(bus.to_string(), "ms2");

  bus.append("k", "g", 0);
  EXPECT_EQ(bus.size(), 1); // adding unit with a zero exponent
  EXPECT_EQ(bus.to_string(), "ms2");
}

TEST(BaseUnits, RemoveZeroExponentsFractions) {

  puq::BaseUnits bus;
  bus.append("m", "s", 2); // adding normal units
  bus.append("c", "m", 1, 2);
  EXPECT_EQ(bus.size(), 2);
  EXPECT_EQ(bus.to_string(), "ms2*cm1:2");

  bus.append("c", "m", -1, 2); // adding inverse unit
  EXPECT_EQ(bus.size(), 1);
  EXPECT_EQ(bus.to_string(), "ms2");

  bus.append("k", "g", 0, 2);
  EXPECT_EQ(bus.size(), 1); // adding unit with a zero exponent
  EXPECT_EQ(bus.to_string(), "ms2");
}

TEST(BaseUnits, Dimensions) {

  puq::BaseUnits bus("km*ns2*mg3"); // units with prefixes
  puq::Dimensions dim = bus.dimensions();
  EXPECT_EQ(dim.to_string(), "1e-24*m*g3*s2");

  bus = puq::BaseUnits("au2"); // derived units
  dim = bus.dimensions();
  EXPECT_EQ(dim.to_string(), "2.23795e22*m2");

  bus = puq::BaseUnits("<v>"); // quantities
  dim = bus.dimensions();
  EXPECT_EQ(dim.to_string(), "m*s-1");
}

TEST(BaseUnits, HasDimensions) {

  puq::BaseUnits bus("kg*m2/s2");
  EXPECT_TRUE(bus.has_dimensions());

  bus = puq::BaseUnits("34*%");
  EXPECT_FALSE(bus.has_dimensions());
}


TEST(BaseUnits, Arithmetics) {

  puq::BaseUnits bus0;
  puq::BaseUnits bus1("kg*m2/s2");
  puq::BaseUnits bus2("kg");

  // multiply by a base unit
  bus0 = bus1 * bus2;
  EXPECT_EQ(bus0.to_string(), "kg2*m2*s-2");

  bus0 = bus1 / bus2;
  EXPECT_EQ(bus0.to_string(), "m2*s-2");

  // multiply by an exponent
  bus0 = puq::math::pow(bus1, 2);
  EXPECT_EQ(bus0.to_string(), "kg2*m4*s-4");

  bus0 = puq::math::pow(bus1, puq::Exponent(1,2));
  EXPECT_EQ(bus0.to_string(), "kg1:2*m*s-1");
  
}
