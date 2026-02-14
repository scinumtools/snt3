#include "pch_tests.h"

using namespace snt;

TEST(Element, Nucleons) {

  mat::Element element("[p]");  
  EXPECT_EQ(element.to_string(), "[p]");
  EXPECT_EQ(element.matterProps.mass.to_string(), "{m_p}");
  EXPECT_EQ(element.elementProps.protons,   1);
  EXPECT_EQ(element.elementProps.neutrons,  0);
  EXPECT_EQ(element.elementProps.electrons, 0);

  element = mat::Element("[n]");
  EXPECT_EQ(element.to_string(), "[n]");
  EXPECT_EQ(element.matterProps.mass.to_string(), "{m_n}");
  EXPECT_EQ(element.elementProps.protons,   0);
  EXPECT_EQ(element.elementProps.neutrons,  1);
  EXPECT_EQ(element.elementProps.electrons, 0);
  
  element = mat::Element("[e]");
  EXPECT_EQ(element.to_string(), "[e]");
  EXPECT_EQ(element.matterProps.mass.to_string(), "{m_e}");
  EXPECT_EQ(element.elementProps.protons,   0);
  EXPECT_EQ(element.elementProps.neutrons,  0);
  EXPECT_EQ(element.elementProps.electrons, 1);

}

TEST(Element, Isotopes) {

  mat::Element element("H{1}");  
  EXPECT_EQ(element.to_string(), "H{1}");
  EXPECT_EQ(element.matterProps.mass.to_string(), "1.00783*u");
  EXPECT_EQ(element.elementProps.protons,   1);
  EXPECT_EQ(element.elementProps.neutrons,  0);
  EXPECT_EQ(element.elementProps.electrons, 1);
  
  element = mat::Element("D");  
  EXPECT_EQ(element.to_string(), "H{2}");
  EXPECT_EQ(element.matterProps.mass.to_string(), "2.0141*u");
  EXPECT_EQ(element.elementProps.protons,   1);
  EXPECT_EQ(element.elementProps.neutrons,  1);
  EXPECT_EQ(element.elementProps.electrons, 1);
  
  element = mat::Element("T");
  EXPECT_EQ(element.to_string(), "H{3}");
  EXPECT_EQ(element.matterProps.mass.to_string(), "3.01605*u");
  EXPECT_EQ(element.elementProps.protons,   1);
  EXPECT_EQ(element.elementProps.neutrons,  2);
  EXPECT_EQ(element.elementProps.electrons, 1);

  element = mat::Element("Mg{24}");
  EXPECT_EQ(element.to_string(), "Mg{24}");
  EXPECT_EQ(element.matterProps.mass.to_string(), "23.985*u");
  EXPECT_EQ(element.elementProps.protons,   12);
  EXPECT_EQ(element.elementProps.neutrons,  12);
  EXPECT_EQ(element.elementProps.electrons, 12);

  element = mat::Element("Hg{202}");
  EXPECT_EQ(element.to_string(), "Hg{202}");
  EXPECT_EQ(element.matterProps.mass.to_string(), "201.971*u");
  EXPECT_EQ(element.elementProps.protons,   80);
  EXPECT_EQ(element.elementProps.neutrons,  122);
  EXPECT_EQ(element.elementProps.electrons, 80);

  element = mat::Element("Cm{245}");
  EXPECT_EQ(element.to_string(), "Cm{245}");
  EXPECT_EQ(element.matterProps.mass.to_string(), "245.065*u");
  EXPECT_EQ(element.elementProps.protons,   96);
  EXPECT_EQ(element.elementProps.neutrons,  149);
  EXPECT_EQ(element.elementProps.electrons, 96);
  
}

TEST(Element, NaturalAbundance) {

  mat::Element element("H");  
  EXPECT_EQ(element.to_string(), "H");
  EXPECT_EQ(element.matterProps.mass.to_string(), "1.00794*u");
  EXPECT_EQ(element.elementProps.protons,   1);
  EXPECT_EQ(element.elementProps.neutrons,  0.000115);
  EXPECT_EQ(element.elementProps.electrons, 1);

  element = mat::Element("Mg");  
  EXPECT_EQ(element.to_string(), "Mg");
  EXPECT_EQ(element.matterProps.mass.to_string(), "24.3051*u");
  EXPECT_EQ(element.elementProps.protons,   12);
  EXPECT_EQ(element.elementProps.neutrons,  12.3202);
  EXPECT_EQ(element.elementProps.electrons, 12);

  element = mat::Element("Hg");
  EXPECT_EQ(element.to_string(), "Hg");
  EXPECT_EQ(element.matterProps.mass.to_string(), "200.599*u");
  EXPECT_EQ(element.elementProps.protons,     80);
  EXPECT_NEAR(element.elementProps.neutrons,  120.629, 1e-3);
  EXPECT_EQ(element.elementProps.electrons,   80);
  
  EXPECT_THROW(mat::Element("Cm"), std::runtime_error);
  
}
