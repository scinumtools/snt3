#include "pch_tests.h"

using namespace snt;

TEST(SolverTemplate, BasicParsing) {

  dip::DIP d;
  d.add_string("foo real = 23 cm");
  d.add_string("bar str = 'baz'");
  dip::Environment env = d.parse();
  dip::TemplateSolver solver(env);
  std::string result = solver.eval("{foo}, {bar}");
  //EXPECT_EQ(result, text_parsed);

}
