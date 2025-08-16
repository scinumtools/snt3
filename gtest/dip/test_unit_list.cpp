#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "dip/dip.h"
#include "dip/environment.h"
#include "dip/nodes/nodes.h"

TEST(UnitList, Keyword) {

  std::string unit_name = "B_lam"; // Spectral Radiance
  std::string unit_definition = "W/(m2*sr*m)";

  dip::DIP d;
  d.add_string("$unit " + unit_name + " = " + unit_definition);
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 0);

  dip::EnvUnit& uenv = env.units.at(unit_name);
  EXPECT_EQ(uenv.name, unit_name);
  EXPECT_EQ(uenv.definition, unit_definition);
}

// TODO: modify puq so that one can set up custom units
