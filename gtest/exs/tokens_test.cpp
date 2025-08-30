#include "pch_tests.h"

// Initialization of the Tokens class
TEST(Tokens, Initialization) {

  exs::Token<exs::Atom> t;

  exs::OperatorList<exs::Atom> operators;

  exs::TokenList<exs::Atom> tokens(&operators);
  EXPECT_EQ(tokens.left.size(), 0);
  EXPECT_EQ(tokens.right.size(), 0);

  tokens.append(exs::ATOM_TOKEN, "3.4");
  EXPECT_EQ(tokens.right.size(), 1);
  EXPECT_EQ(tokens.right.front().atom->to_string(), "3.4");
}

// Test get and put tokenss
TEST(Tokens, GetAndPut) {

  exs::OperatorList<exs::Atom> operators;

  // prepare tokens
  exs::TokenList<exs::Atom> tokens(&operators);

  // test empty arrays
  EXPECT_EQ(tokens.get_left().type, exs::EMPTY_TOKEN);
  EXPECT_EQ(tokens.get_right().type, exs::EMPTY_TOKEN);

  // fill with some dummy values
  exs::AtomList<exs::Atom> atoms;
  int nitems = 4;
  for (int i = 0; i < nitems; i++) {
    tokens.append(exs::ATOM_TOKEN, std::to_string(i));
  }

  // pass 2 strings from right to left
  exs::Token token = tokens.get_right();
  tokens.put_left(token);
  EXPECT_EQ(token.atom->to_string(), "0");

  token = tokens.get_right();
  tokens.put_left(token);
  EXPECT_EQ(token.atom->to_string(), "1");
  EXPECT_EQ(tokens.right.size(), nitems - 2);
  EXPECT_EQ(tokens.left.size(), 2);

  // make sure that items are selected in a correct order
  token = tokens.get_left();
  tokens.put_right(token);
  EXPECT_EQ(token.atom->to_string(), "1");
  EXPECT_EQ(tokens.right.size(), nitems - 1);
  EXPECT_EQ(tokens.left.size(), 1);
}
