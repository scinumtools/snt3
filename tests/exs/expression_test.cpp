#include "pch_tests.h"
#include <snt/exs/expression.h>

using namespace snt;

// Initialization of the Expression class
TEST(Expression, Initialization) {

  exs::Expression e("Hello + World");

  EXPECT_EQ(e.left, "");
  EXPECT_EQ(e.right, "Hello + World");
  EXPECT_EQ(e.expr, "Hello + World");
}

// Test correct print statement
TEST(Expression, Print) {

  exs::Expression e("Hello + World");

  testing::internal::CaptureStdout();
  e.print();
  std::string output = testing::internal::GetCapturedStdout();
  EXPECT_EQ(output, "Expression(L'' R'Hello + World' E'Hello + World')\n");
}

// Manipulation of the text
TEST(Expression, Methods) {

  exs::Expression e("Hello + World");

  e.shift(6);
  EXPECT_EQ(e.left, "Hello ");
  EXPECT_EQ(e.right, "+ World");

  std::string l = e.pop_left();
  EXPECT_EQ(l, "Hello");
  EXPECT_EQ(e.left, "");
  EXPECT_EQ(e.right, "+ World");
}
