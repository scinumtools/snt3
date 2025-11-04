#include "pch_tests.h"

using namespace snt;

TEST(Element, Nucleons) {

  mat::Element element("[p]");  
  EXPECT_EQ(element.to_string(), "[p]");
  EXPECT_EQ(element.protons,   1);
  EXPECT_EQ(element.neutrons,  0);
  EXPECT_EQ(element.electrons, 0);

  element = mat::Element("[n]");
  EXPECT_EQ(element.to_string(), "[n]");
  EXPECT_EQ(element.protons,   0);
  EXPECT_EQ(element.neutrons,  1);
  EXPECT_EQ(element.electrons, 0);
  
  element = mat::Element("[e]");
  EXPECT_EQ(element.to_string(), "[e]");
  EXPECT_EQ(element.protons,   0);
  EXPECT_EQ(element.neutrons,  0);
  EXPECT_EQ(element.electrons, 1);

}

TEST(Element, HydrogenIsotopes) {

  mat::Element element("H{1}");  
  EXPECT_EQ(element.to_string(), "H{1}");
  EXPECT_EQ(element.protons,   1);
  EXPECT_EQ(element.neutrons,  0);
  EXPECT_EQ(element.electrons, 1);
  
  element = mat::Element("D");  
  EXPECT_EQ(element.to_string(), "H{2}");
  EXPECT_EQ(element.protons,   1);
  EXPECT_EQ(element.neutrons,  1);
  EXPECT_EQ(element.electrons, 1);
  
  element = mat::Element("T");
  EXPECT_EQ(element.to_string(), "H{3}");
  EXPECT_EQ(element.protons,   1);
  EXPECT_EQ(element.neutrons,  2);
  EXPECT_EQ(element.electrons, 1);

}

TEST(Element, NaturalAbundance) {

  mat::Element element("Mg");  
  EXPECT_EQ(element.to_string(), "Mg");
  EXPECT_EQ(element.mass.to_string(), "24.3051*Da");
  EXPECT_EQ(element.protons,   12);
  EXPECT_EQ(element.neutrons,  12.3202);
  EXPECT_EQ(element.electrons, 12);

}
