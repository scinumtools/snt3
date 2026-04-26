#include "commands.h"

#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <snt/puq/calc/calculator.h>
#include <snt/puq/lists.h>
#include <sstream>
#include <stdexcept>

namespace snt::cli {

  std::string get_expression(std::deque<std::string>& arguments) {
    if (arguments.size()) {
      std::string expr = arguments.front();
      arguments.pop_front();
      return expr;
    } else {
      return "";
    }
  }

  puq::SystemType get_system(std::deque<std::string>& arguments) {
    for (auto sys : puq::SystemMap) {
      if (sys.second->SystemAbbrev == arguments.front()) {
	arguments.pop_front();
	return sys.first;
      }
    }
    return puq::SystemType::NONE;
  }

  void change_system(puq::UnitSystem& us, std::deque<std::string>& arguments) {
    puq::SystemType system = get_system(arguments);
    if (system != puq::SystemType::NONE)
      us.change(system);
  }

  void convert_quantity(puq::Quantity& q, std::deque<std::string>& arguments) {
    puq::SystemType sys2 = get_system(arguments);
    std::string expr2 = get_expression(arguments);
    std::string quant = get_expression(arguments);
    if (quant == "") {
      if (sys2 == puq::SystemType::NONE)
	q = q.convert(expr2);
      else
	q = q.convert(expr2, sys2);
    } else {
      if (sys2 == puq::SystemType::NONE)
	q = q.convert(expr2, puq::UnitSystem::System, quant);
      else {
	q = q.convert(expr2, sys2, quant);
      }
    }
  }

  void command_solve(std::deque<std::string>& arguments) {
    std::string expr1 = get_expression(arguments);
    puq::Calculator calc;
    puq::Quantity q = calc.solve(expr1).value;
    if (arguments[0] != "")
      convert_quantity(q, arguments);
    std::cout << q.to_string() << '\n';
  }

  void command_convert(std::deque<std::string>& arguments) {
    puq::SystemType sys1 = get_system(arguments);
    std::string expr1 = get_expression(arguments);
    puq::Quantity q;
    if (sys1 == puq::SystemType::NONE)
      q = puq::Quantity(expr1);
    else
      q = puq::Quantity(expr1, sys1);
    convert_quantity(q, arguments);
    std::cout << q.to_string() << '\n';
  }
  
}
