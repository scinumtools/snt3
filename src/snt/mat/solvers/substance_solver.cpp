#include "substance_solver.h"
#include "../substance.h"
#include "../settings.h"

namespace snt::mat {

  SubstanceSolver::SubstanceSolver() {

    exs::OperatorList<Substance> operators;
    operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<exs::OperatorParentheses<Substance>>());
    operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply<Substance>>(std::string(MAT_SYMBOL_MULTIPLY)));
    operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<Substance>>(std::string(MAT_SYMBOL_PLUS)));

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION,  {exs::PARENTHESES_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR});
    
    solver = std::make_unique<exs::Solver<Substance>>(operators, steps);
  }

  std::string SubstanceSolver::preprocess(const std::string& expr) {
    //std::cout << expr << "\n";
    std::stringstream ss;
    ParserState state = ParserState::START;
    for (char c: expr) {
      switch (state) {
      case ParserState::START:
	//std::cout << c << " start\n";
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
	//std::cout << c << " elem\n";
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
	} else if (c==')') {
	  state = ParserState::END;
	  ss << c;
	} else {
	  state = ParserState::ERROR;
	}
	break;
      case ParserState::STATE:
	//std::cout << c << " state\n";
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
	//std::cout << c << " end\n";
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
	//std::cout << c << " oper\n";
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

  Substance SubstanceSolver::solve(const std::string& expr) {

    std::string pexpr = preprocess(expr);
    
    Substance substance = solver->solve(pexpr);
    
    return substance;
  }

}
