#include "pch_tests.h"

#include <memory>

// Initialization of the operators map
TEST(Operators, Initialization) {

  exs::OperatorList<exs::Atom> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<exs::Atom>>());
  operators.append(exs::SUBTRACT_OPERATOR, std::make_shared<exs::OperatorSubtract<exs::Atom>>());

  exs::OperatorBase<exs::Atom>* op = operators.select(exs::ADD_OPERATOR);
  EXPECT_EQ(op->type, exs::ADD_OPERATOR);

  op = operators.select(exs::SUBTRACT_OPERATOR);
  EXPECT_EQ(op->type, exs::SUBTRACT_OPERATOR);
}

TEST(Operators, CustomOperatorsList) {

  // initialize operators
  exs::OperatorList<exs::Atom> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<exs::Atom>>());
  operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply<exs::Atom>>());

  exs::OperatorBase<exs::Atom>* op = operators.select(exs::MULTIPLY_OPERATOR);
  EXPECT_EQ(op->type, exs::MULTIPLY_OPERATOR);
}

TEST(Operators, OperatorAdd) {

  exs::OperatorList<exs::Atom> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<exs::Atom>>());

  // prepare tokens
  exs::TokenList<exs::Atom> tokens(&operators);
  tokens.append(exs::OPERATOR_TOKEN, exs::ADD_OPERATOR);
  tokens.append(exs::OPERATOR_TOKEN, exs::ADD_OPERATOR);
  tokens.append(exs::ATOM_TOKEN, "3.4");
  // tokens.print(true);

  // extract first token on the right
  tokens.get_right();

  // get an operator and perform unary operation on the remaining tokens
  exs::OperatorBase<exs::Atom>* op = operators.select(exs::ADD_OPERATOR);
  // tokens.print(true);
  op->operate_unary(&tokens);
  // tokens.print(true);
}

TEST(Operators, OpearateAdd) {

  exs::OperatorList<exs::Atom> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<exs::Atom>>());

  // prepare tokens
  exs::TokenList<exs::Atom> tokens(&operators);
  tokens.append(exs::OPERATOR_TOKEN, exs::ADD_OPERATOR);
  tokens.append(exs::OPERATOR_TOKEN, exs::ADD_OPERATOR);
  tokens.append(exs::ATOM_TOKEN, "3.4");
  // tokens.print(true);

  // create operator list
  std::vector<int> oper = {exs::ADD_OPERATOR};

  // operate on tokens
  tokens.operate(oper, exs::UNARY_OPERATION);
  // tokens.print(true);
}
