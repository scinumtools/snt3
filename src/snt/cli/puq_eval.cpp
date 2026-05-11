#include "puq_eval.h"

#include <snt/puq/calc/calculator.h>

namespace snt::cli {

  void PUQEval::argument_input_system(const std::string& system) {
    for (auto sys : puq::SystemMap) {
      if (sys.second->SystemAbbrev == system) {
	input_system = sys.first;
	return;
      }
    }
    throw std::runtime_error("Could not find unit system: "+system);
  }
  
  void PUQEval::argument_output_system(const std::string& system) {
    for (auto sys : puq::SystemMap) {
      if (sys.second->SystemAbbrev == system) {
	output_system = sys.first;
	return;
      }
    }
    throw std::runtime_error("Could not find unit system: "+system);
  }

  void PUQEval::argument_output_units(const std::string& ounits) {
    output_units = ounits;
  }
  
  void PUQEval::argument_output_quantity(const std::string& oquantity) {
    output_quantity = oquantity;
  }
  
  void PUQEval::execute() {
    
    puq::UnitSystem us(puq::SystemType::SI);

    if (input_system != puq::SystemType::NONE) {
      us.change(input_system);
    }
        
    puq::Calculator calc;
    puq::Quantity q = calc.solve(expression).value;
    if (!output_quantity.empty()) {
      if (output_quantity == "") {
	if (output_system == puq::SystemType::NONE)
	  q = q.convert(output_units);
	else
	  q = q.convert(output_units, output_system);
      } else {
	if (output_system == puq::SystemType::NONE)
	  q = q.convert(output_units, puq::UnitSystem::System, output_quantity);
	else {
	  q = q.convert(output_units, output_system, output_quantity);
	}
      }
      
    }
    
    std::cout << q.to_string() << '\n';
  }
    
}
