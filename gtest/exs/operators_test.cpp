#include "pch_tests.h"

#include <memory>

using namespace snt;

// Initialization of the operators map
TEST(Operators, Initialization) {

  exs::OperatorList<> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<>>());
  operators.append(exs::SUBTRACT_OPERATOR, std::make_shared<exs::OperatorSubtract<>>());

  exs::OperatorBase<>* op = operators.select(exs::ADD_OPERATOR);
  EXPECT_EQ(op->type, exs::ADD_OPERATOR);

  op = operators.select(exs::SUBTRACT_OPERATOR);
  EXPECT_EQ(op->type, exs::SUBTRACT_OPERATOR);
}

TEST(Operators, CustomOperatorsList) {

  // initialize operators
  exs::OperatorList<> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<>>());
  operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply<>>());

  exs::OperatorBase<>* op = operators.select(exs::MULTIPLY_OPERATOR);
  EXPECT_EQ(op->type, exs::MULTIPLY_OPERATOR);
}

TEST(Operators, OperatorAdd) {

  exs::OperatorList<> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<>>());

  // prepare tokens
  exs::TokenList<> tokens(&operators);
  tokens.append(exs::OPERATOR_TOKEN, exs::ADD_OPERATOR);
  tokens.append(exs::OPERATOR_TOKEN, exs::ADD_OPERATOR);
  exs::Atom atom = exs::Atom::from_string("3.4");
  tokens.append(exs::ATOM_TOKEN, std::make_unique<exs::Atom>(atom));
  // tokens.print(true);

  // extract first token on the right
  tokens.get_right();

  // get an operator and perform unary operation on the remaining tokens
  exs::OperatorBase<>* op = operators.select(exs::ADD_OPERATOR);
  // tokens.print(true);
  op->operate_unary(&tokens);
  // tokens.print(true);
}

TEST(Operators, OpearateAdd) {

  exs::OperatorList<> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<>>());

  // prepare tokens
  exs::TokenList<> tokens(&operators);
  tokens.append(exs::OPERATOR_TOKEN, exs::ADD_OPERATOR);
  tokens.append(exs::OPERATOR_TOKEN, exs::ADD_OPERATOR);
  exs::Atom atom = exs::Atom::from_string("3.4");
  tokens.append(exs::ATOM_TOKEN, std::make_unique<exs::Atom>(atom));
  // tokens.print(true);

  // create operator list
  std::vector<int> oper = {exs::ADD_OPERATOR};

  // operate on tokens
  tokens.operate(oper, exs::UNARY_OPERATION);
  // tokens.print(true);
}
