#include "pch_tests.h"

using namespace snt;

TEST(SubstanceSolver, Preprocess) {

  mat::SubstanceSolver solver;  

  // implicit multiplication and addition
  EXPECT_EQ(solver.preprocess("C2B4"), "C * 2 + B * 4");
  EXPECT_EQ(solver.preprocess("C2 B4"), "C * 2 + B * 4");

  // isotopes and ionisation
  EXPECT_EQ(solver.preprocess("C{13+2}B{11}H{-}2"), "C{13+2} + B{11} + H{-} * 2");

  // parentheses
  EXPECT_EQ(solver.preprocess("(CB2)2"),                         "(C + B * 2) * 2");
  EXPECT_EQ(solver.preprocess("((CB2)2Al)3"),                    "((C + B * 2) * 2 + Al) * 3");
  EXPECT_EQ(solver.preprocess("C{13+2}(B{11}Li2)4 H{-}2 O{+3}"), "C{13+2} + (B{11} + Li * 2) * 4 + H{-} * 2 + O{+3}");
  EXPECT_EQ(solver.preprocess("Mg(NO3)2"),                       "Mg + (N + O * 3) * 2");

  // preprocessed expressions are constant
  EXPECT_EQ(solver.preprocess("(C + B * 2) * 2"),             "(C + B * 2) * 2");
  EXPECT_EQ(solver.preprocess("C{13+2} + (B{11} + Li * 2)4"), "C{13+2} + (B{11} + Li * 2) * 4");
    
}

TEST(SubstanceSolver, Solver) {

  mat::SubstanceSolver solver;  

  EXPECT_EQ(solver.solve("C"),        mat::ElementMap({{"C", 1}}));
  EXPECT_EQ(solver.solve("C{13}2"),   mat::ElementMap({{"C{13}", 2}}));
  EXPECT_EQ(solver.solve("C2Be3"),    mat::ElementMap({{"C", 2},{"Be",3}}));
  EXPECT_EQ(solver.solve("Mg(NO3)2"), mat::ElementMap({{"Mg", 1},{"N",2},{"O",6}}));
  //EXPECT_EQ(solver.solve("C").to_string(),      "Substance(mass=12.011 Z=6 N=6.011 e=6)");
  //EXPECT_EQ(solver.solve("C{12}"),  "Substance(mass=12.000 Z=6 N=6.000 e=6)");
  //EXPECT_EQ(solver.solve("C{-2}"),  "Substance(mass=12.010 Z=6 N=6.011 e=4)");
  //EXPECT_EQ(solver.solve("C{+}"),   "Substance(mass=12.011 Z=6 N=6.011 e=7)");
  //EXPECT_EQ(solver.solve("C{12-}"), "Substance(mass=11.999 Z=6 N=6.000 e=5)");
  //EXPECT_EQ(solver.solve("C{13}"),  "Substance(mass=13.003 Z=6 N=7.000 e=6)");
  //EXPECT_EQ(solver.solve("C{13-2}"),"Substance(mass=13.002 Z=6 N=7.000 e=4)");

}
