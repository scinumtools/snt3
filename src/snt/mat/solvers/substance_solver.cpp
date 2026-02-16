#include <snt/mat/solvers/substance_solver.h>
#include <snt/mat/substance.h>
#include <snt/mat/settings.h>

namespace snt::mat {

  std::unique_ptr<exs::Solver<SubstanceAtom>> SubstanceSolver::solver = []{
    exs::OperatorList operators;
    operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<exs::OperatorParentheses>());
    operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>(std::string(MAT_SYMBOL_MULTIPLY)));
    operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd>(std::string(MAT_SYMBOL_PLUS)));

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION,  {exs::PARENTHESES_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR});
    
    return std::make_unique<exs::Solver<SubstanceAtom>>(operators, steps);
  }();
  
  std::string SubstanceSolver::preprocess(const std::string& expr) {
    if constexpr (Config::debug_substance_solver) {
      std::cout << expr << "\n";
    }
    std::stringstream ss;
    ParserState state = ParserState::START;
    for (char c: expr) {
      switch (state) {
      case ParserState::START:
	if constexpr (Config::debug_substance_solver) {
	  std::cout << c << " start\n";
	}
	if (c=='(') {
	  ss << c;
	} else if (std::isupper(c)) {
	  state = ParserState::ELEMENT;
	  ss << c;
	} else if (c!=' ') {
	  state = ParserState::ERROR;
	}
	break;
      case ParserState::ELEMENT:
	if constexpr (Config::debug_substance_solver) {
	  std::cout << c << " elem\n";
	}
	if (std::isupper(c)) {
	  ss << " + " << c;
	} else if (std::islower(c)) {
	  ss << c;
	} else if (c==' ') {
	  state = ParserState::END;
	} else if (c=='{') {
	  state = ParserState::STATE;
	  ss << c;
	} else if (std::isdigit(c)) {
	  state = ParserState::END;
	  ss << " * " << c;
	} else if (c=='(') {
	  state = ParserState::START;
	  ss << " + " << c;
	} else if (c==')') {
	  state = ParserState::END;
	  ss << c;
	} else {
	  state = ParserState::ERROR;
	}
	break;
      case ParserState::STATE:
	if constexpr (Config::debug_substance_solver) {
	  std::cout << c << " state\n";
	}
	if (std::isdigit(c) || c=='-' || c=='+') {
	  ss << c;
	} else if (c=='}') {
	  state = ParserState::END;
	  ss << c;
	} else {
	  state = ParserState::ERROR;	  
	}
	break;
      case ParserState::END:
	if constexpr (Config::debug_substance_solver) {
	  std::cout << c << " end\n";
	}
	if (c==')') {
	  ss << c;
	} else if (std::isdigit(c)) {
	  ss << " * " << c;
	} else if (std::isupper(c)) {
	  state = ParserState::ELEMENT;
	  ss << " + " << c;
	} else if (c=='(') {
	  state = ParserState::START;
	  ss << " + " << c;
	} else if (c=='*' || c=='+') {
	  state = ParserState::OPERATION;
	  ss << ' ' << c << ' ';
	} else if (c!=' ') {
	  state = ParserState::ERROR;
	}
	break;
      case ParserState::OPERATION:
	if constexpr (Config::debug_substance_solver) {
	  std::cout << c << " oper\n";
	}
	if (std::isdigit(c)) {
	  ss << c;
	} else if (c==')') {
	  state = ParserState::END;
	  ss << c;
	} else if (std::isupper(c)) {
	  state = ParserState::ELEMENT;
	  ss << c;
	} else if (c=='(') {
	  state = ParserState::START;
	  ss <<  c;
	} else if (c!=' ') {
	  state = ParserState::ERROR;
	}
	break;
      case ParserState::ERROR:
	throw std::runtime_error("Invalid substance expression on: "+expr);
	break;
      }
    }
    return ss.str();
  }

  ElementMap SubstanceSolver::solve(const std::string& expr) {

    std::string pexpr = preprocess(expr);

    SubstanceAtom atom = solver->solve(pexpr);

    return atom.value;
  }

}
