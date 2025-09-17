#include "pch_tests.h"

using namespace snt;

TEST(SolverTemplate, BasicParsing) {

  dip::DIP d;
  d.add_string("foo real = 23 cm");
  d.add_string("bar str = 'baz'");
  dip::Environment env = d.parse();

  dip::TemplateSolver solver(env);
  std::string result = solver.eval("{{?foo}}, {{?bar}}");
  EXPECT_EQ(result, "23 cm, 'baz'");

}

TEST(SolverTemplate, Formatting) {

  dip::DIP d;
  d.add_string("foo real = 1.23e4 cm");
  dip::Environment env = d.parse();
  
  dip::TemplateSolver solver(env);
  std::string result;

  result = solver.eval("foo: {{?foo}}");
  EXPECT_EQ(result, "foo: 1.23e4 cm");

  result = solver.eval("foo: {{?foo}:.03e}");
  EXPECT_EQ(result, "foo: 1.23e4 cm");
  
}
