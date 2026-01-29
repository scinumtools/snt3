#ifndef EXS_SOLVER_H
#define EXS_SOLVER_H

#include "branching/condition.h"
#include "branching/parentheses.h"
#include "comparison/equal.h"
#include "comparison/greater.h"
#include "comparison/greater_equal.h"
#include "comparison/less.h"
#include "comparison/less_equal.h"
#include "comparison/not_equal.h"
#include "logical/and.h"
#include "logical/not.h"
#include "logical/or.h"
#include "math/add.h"
#include "math/cosinus.h"
#include "math/cubic_root.h"
#include "math/divide.h"
#include "math/exponent.h"
#include "math/logarithm.h"
#include "math/logarithm_10.h"
#include "math/logarithm_base.h"
#include "math/modulo.h"
#include "math/multiply.h"
#include "math/power.h"
#include "math/power_base.h"
#include "math/sinus.h"
#include "math/square_root.h"
#include "math/subtract.h"
#include "math/tangens.h"
#include "settings.h"
#include "step_list.h"
#include "token_list.h"
#include "../settings.h"

#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class Solver {
    S* settings;

  public:
    OperatorList<S> operators;
    StepList steps;

    // constructors without settings
    Solver(S* set = {}) : settings(set) {
      init_steps();
      init_operators();
    };
    Solver(OperatorList<S>& o, S* set = {}) : operators(o), settings(set) {
      init_steps();
    };
    Solver(StepList& s, S* set = {}) : steps(s), settings(set) {
      init_operators();
    };
    Solver(OperatorList<S>& o, StepList& s, S* set = {}) : operators(o), steps(s), settings(set) {};

    // solve expressions
    A solve(std::string expression) {
      Expression expr(expression);
      //CHECKPOINT( expr.to_string() );
      TokenList<S> tokens(&operators, settings);

      // Tokenize expression
      while (expr.right.length() > 0) {
        bool is_operator = false;
        for (auto o : operators.order) {
          OperatorBase<S>* op = operators.select(o);
          //CHECKPOINT( op->symbol );
          if (op->check(expr)) {
            is_operator = true;
            std::string left = expr.pop_left();
            if (left.length() > 0) {
	      A atom = A::from_string(left, settings);
              tokens.append(ATOM_TOKEN, std::make_unique<A>(std::move(atom.value)));
            }
            op->parse(expr);
            if (op->groups.size() > 0) {
              std::vector<std::string> groups = op->groups;
              // CHECKPOINT( groups.size() )
              for (auto e : groups) {
		A atom = solve(e);
                tokens.append(ATOM_TOKEN, std::make_unique<A>(std::move(atom.value)));
              }
            }
            tokens.append(OPERATOR_TOKEN, op->type);
            //CHECKPOINT( expr.to_string() );
          }
        }
        if (is_operator == false) {
          expr.shift();
          //CHECKPOINT( expr.to_string() );
        }
        //CHECKPOINT( tokens.to_string(true) );
      }
      std::string left = expr.pop_left();
      if (left.length() > 0) {
	A atom = A::from_string(left, settings);
	tokens.append(ATOM_TOKEN, std::make_unique<A>(std::move(atom.value)));
      }
      //CHECKPOINT( expr.to_string() );

      for (auto s : steps.steps) {
        //CHECKPOINT( tokens.to_string(true) );
        tokens.operate(s.second, s.first);
      }
      //CHECKPOINT( tokens.to_string(true) );

      if (tokens.left.size() > 0 or tokens.right.size() > 1) {
        //CHECKPOINT( tokens.to_string(true) );
        throw std::logic_error("Cannot solve expression due to unprocessed tokens: " + tokens.to_string(true));
      }

      Token token = tokens.get_right();
      // std::cout << &token << " " << token.atom << " " << token.atom->value << std::endl;
      if (!token.atom)
        throw std::runtime_error("Cannot dereference a null atom pointer");
      A* catom = static_cast<A*>(token.atom);
      return A(std::move(catom->value));
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
      operators.append(TANGENS_OPERATOR, std::make_shared<OperatorTangens<S>>());
      operators.append(COSINUS_OPERATOR, std::make_shared<OperatorCosinus<S>>());
      operators.append(SINUS_OPERATOR, std::make_shared<OperatorSinus<S>>());
      operators.append(CUBIC_ROOT_OPERATOR, std::make_shared<OperatorCubicRoot<S>>());
      operators.append(SQUARE_ROOT_OPERATOR, std::make_shared<OperatorSquareRoot<S>>());
      operators.append(POWER_BASE_OPERATOR, std::make_shared<OperatorPowerBase<S>>());
      operators.append(LOGARITHM_BASE_OPERATOR, std::make_shared<OperatorLogarithmBase<S>>());
      operators.append(LOGARITHM_10_OPERATOR, std::make_shared<OperatorLogarithm10<S>>());
      operators.append(LOGARITHM_OPERATOR, std::make_shared<OperatorLogarithm<S>>());
      operators.append(EXPONENT_OPERATOR, std::make_shared<OperatorExponent<S>>());
      operators.append(PARENTHESES_OPERATOR, std::make_shared<OperatorParentheses<S>>()); // should be last of group operators

      operators.append(CONDITION_OPERATOR, std::make_shared<OperatorCondition<S>>());

      operators.append(POWER_OPERATOR, std::make_shared<OperatorPower<S>>());

      operators.append(MODULO_OPERATOR, std::make_shared<OperatorModulo<S>>());
      operators.append(MULTIPLY_OPERATOR, std::make_shared<OperatorMultiply<S>>());
      operators.append(DIVIDE_OPERATOR, std::make_shared<OperatorDivide<S>>());
      operators.append(ADD_OPERATOR, std::make_shared<OperatorAdd<S>>());
      operators.append(SUBTRACT_OPERATOR, std::make_shared<OperatorSubtract<S>>());

      operators.append(EQUAL_OPERATOR, std::make_shared<OperatorEqual<S>>());
      operators.append(NOT_EQUAL_OPERATOR, std::make_shared<OperatorNotEqual<S>>());

      operators.append(NOT_OPERATOR, std::make_shared<OperatorNot<S>>()); // needs to be after NOT_EQUAL

      operators.append(LESS_EQUAL_OPERATOR, std::make_shared<OperatorLessEqual<S>>());
      operators.append(GREATER_EQUAL_OPERATOR, std::make_shared<OperatorGreaterEqual<S>>());
      operators.append(LESS_OPERATOR, std::make_shared<OperatorLess<S>>());
      operators.append(GREATER_OPERATOR, std::make_shared<OperatorGreater<S>>());

      operators.append(AND_OPERATOR, std::make_shared<OperatorAnd<S>>());
      operators.append(OR_OPERATOR, std::make_shared<OperatorOr<S>>());
    }
  };

} // namespace snt::exs

#endif // EXS_SOLVER_H
