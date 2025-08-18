#ifndef EXS_SOLVER_H
#define EXS_SOLVER_H

#include "settings.h"
#include "step_list.h"
#include "token_list.h"

#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace exs {

  template <class A, typename S = EmptySettings>
  class Solver {
    S settings;

  public:
    OperatorList<A, S> operators;
    StepList steps;

    // constructors without settings
    Solver(S set = {}) : settings(set) {
      init_steps();
      init_operators();
    };
    Solver(OperatorList<A, S>& o, S set = {}) : operators(o), settings(set) {
      init_steps();
    };
    Solver(StepList& s, S set = {}) : steps(s), settings(set) {
      init_operators();
    };
    Solver(OperatorList<A, S>& o, StepList& s, S set = {}) : operators(o), steps(s), settings(set) {};

    // solve expressions
    A solve(std::string expression) {
      Expression expr(expression);
      // expr.print();
      TokenList<A, S> tokens(&operators, &settings);

      // Tokenize expression
      while (expr.right.length() > 0) {
        bool is_operator = false;
        for (auto o : operators.order) {
          OperatorBase<A, S>* op = operators.select(o);
          // std::cout << op->symbol << std::endl;
          if (op->check(expr)) {
            is_operator = true;
            std::string left = expr.pop_left();
            if (left.length() > 0) {
              tokens.append(ATOM_TOKEN, left);
            }
            op->parse(expr);
            if (op->groups.size() > 0) {
              std::vector<std::string> groups = op->groups;
              // std::cout <<  groups.size() << std::endl;
              for (auto e : groups) {
                tokens.append(ATOM_TOKEN, solve(e));
              }
            }
            tokens.append(OPERATOR_TOKEN, op->type);
            // expr.print();
          }
        }
        if (is_operator == false) {
          expr.shift();
          // expr.print();
        }
        // tokens.print(true);
      }
      std::string left = expr.pop_left();
      if (left.length() > 0) {
        tokens.append(ATOM_TOKEN, left);
      }
      // expr.print();

      for (auto s : steps.steps) {
        // tokens.print(true);
        tokens.operate(s.second, s.first);
      }
      // tokens.print(true);

      if (tokens.left.size() > 0 or tokens.right.size() > 1) {
        // tokens.print(true);
        throw std::logic_error("Cannot solve expression due to unprocessed tokens: " + tokens.to_string(true));
      }

      Token token = tokens.get_right();
      // std::cout << &token << " " << token.atom << " " << token.atom->value << std::endl;
      if (!token.atom)
        throw std::runtime_error("Cannot dereference a null atom pointer");
      return *token.atom;
    };

  private:
    void init_steps() {
      steps.append(GROUP_OPERATION, {EXPONENT_OPERATOR, LOGARITHM_OPERATOR, LOGARITHM_10_OPERATOR,
                                     LOGARITHM_BASE_OPERATOR, POWER_BASE_OPERATOR,
                                     SQUARE_ROOT_OPERATOR, CUBIC_ROOT_OPERATOR,
                                     SINUS_OPERATOR, COSINUS_OPERATOR, TANGENS_OPERATOR});
      steps.append(GROUP_OPERATION, {PARENTHESES_OPERATOR});
      steps.append(UNARY_OPERATION, {ADD_OPERATOR, SUBTRACT_OPERATOR});
      steps.append(TERNARY_OPERATION, {CONDITION_OPERATOR});
      steps.append(BINARY_OPERATION, {POWER_OPERATOR});
      steps.append(BINARY_OPERATION, {MULTIPLY_OPERATOR, DIVIDE_OPERATOR, MODULO_OPERATOR});
      steps.append(BINARY_OPERATION, {ADD_OPERATOR, SUBTRACT_OPERATOR});
      steps.append(BINARY_OPERATION, {LESS_EQUAL_OPERATOR, GREATER_EQUAL_OPERATOR,
                                      LESS_OPERATOR, GREATER_OPERATOR});
      steps.append(BINARY_OPERATION, {EQUAL_OPERATOR, NOT_EQUAL_OPERATOR});
      steps.append(UNARY_OPERATION, {NOT_OPERATOR});
      steps.append(BINARY_OPERATION, {AND_OPERATOR});
      steps.append(BINARY_OPERATION, {OR_OPERATOR});
    }
    void init_operators() {
      operators.append(TANGENS_OPERATOR, std::make_shared<OperatorTangens<A, S>>());
      operators.append(COSINUS_OPERATOR, std::make_shared<OperatorCosinus<A, S>>());
      operators.append(SINUS_OPERATOR, std::make_shared<OperatorSinus<A, S>>());
      operators.append(CUBIC_ROOT_OPERATOR, std::make_shared<OperatorCubicRoot<A, S>>());
      operators.append(SQUARE_ROOT_OPERATOR, std::make_shared<OperatorSquareRoot<A, S>>());
      operators.append(POWER_BASE_OPERATOR, std::make_shared<OperatorPowerBase<A, S>>());
      operators.append(LOGARITHM_BASE_OPERATOR, std::make_shared<OperatorLogarithmBase<A, S>>());
      operators.append(LOGARITHM_10_OPERATOR, std::make_shared<OperatorLogarithm10<A, S>>());
      operators.append(LOGARITHM_OPERATOR, std::make_shared<OperatorLogarithm<A, S>>());
      operators.append(EXPONENT_OPERATOR, std::make_shared<OperatorExponent<A, S>>());
      operators.append(PARENTHESES_OPERATOR, std::make_shared<OperatorParentheses<A, S>>()); // should be last of group operators

      operators.append(CONDITION_OPERATOR, std::make_shared<OperatorCondition<A, S>>());

      operators.append(POWER_OPERATOR, std::make_shared<OperatorPower<A, S>>());

      operators.append(MODULO_OPERATOR, std::make_shared<OperatorModulo<A, S>>());
      operators.append(MULTIPLY_OPERATOR, std::make_shared<OperatorMultiply<A, S>>());
      operators.append(DIVIDE_OPERATOR, std::make_shared<OperatorDivide<A, S>>());
      operators.append(ADD_OPERATOR, std::make_shared<OperatorAdd<A, S>>());
      operators.append(SUBTRACT_OPERATOR, std::make_shared<OperatorSubtract<A, S>>());

      operators.append(EQUAL_OPERATOR, std::make_shared<OperatorEqual<A, S>>());
      operators.append(NOT_EQUAL_OPERATOR, std::make_shared<OperatorNotEqual<A, S>>());

      operators.append(NOT_OPERATOR, std::make_shared<OperatorNot<A, S>>()); // needs to be after NOT_EQUAL

      operators.append(LESS_EQUAL_OPERATOR, std::make_shared<OperatorLessEqual<A, S>>());
      operators.append(GREATER_EQUAL_OPERATOR, std::make_shared<OperatorGreaterEqual<A, S>>());
      operators.append(LESS_OPERATOR, std::make_shared<OperatorLess<A, S>>());
      operators.append(GREATER_OPERATOR, std::make_shared<OperatorGreater<A, S>>());

      operators.append(AND_OPERATOR, std::make_shared<OperatorAnd<A, S>>());
      operators.append(OR_OPERATOR, std::make_shared<OperatorOr<A, S>>());
    }
  };

} // namespace exs

#endif // EXS_SOLVER_H
