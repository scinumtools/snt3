#include "puq_info.h"
#include "snt/puq/to_string.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <snt/puq/base_units.h>
#include <snt/puq/dimensions.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/util/data_table.h>

namespace snt::cli {

  void PUQInfo::argument_input_system(const std::string& system) {
    for (auto sys : puq::SystemMap) {
      if (sys.second->SystemAbbrev == system) {
	input_system = sys.first;
	return;
      }
    }
    throw std::runtime_error("Could not find unit system: "+system);
  }
  
  void PUQInfo::execute() {
    
    puq::UnitSystem us(puq::SystemType::SI);

    if (input_system != puq::SystemType::NONE) {
      us.change(input_system);
    }

    puq::Measurement uv = puq::Quantity(expression).measurement;
    puq::BaseUnits bus = uv.baseunits;
    puq::Dimensions dim = bus.dimensions();
    puq::Dimensions dim_m = bus.dimensions();
    dim_m.numerical *= uv.result;
    std::cout << '\n'
	      << "Expression:  " << expression << '\n'
	      << '\n';
    std::cout << "Unit system: " << puq::UnitSystem::Data->SystemAbbrev << " (" << puq::UnitSystem::Data->SystemName << ")" << '\n';
    std::cout << "Result:   " << puq::to_string(uv.result) << '\n';
    std::cout << "Base units:  " << puq::to_string(uv.baseunits) << '\n';
    std::stringstream ss;
    bool display = false;
    for (const auto& unit : puq::UnitSystem::Data->DimensionMap) {
      if (puq::Dimensions(1, unit.second.dimensions) != dim)
	continue;
      ss << (display ? ", " : "") << unit.first;
      display = true;
    }
    if (display) {
      std::cout << '\n';
      std::cout << "Conversions: " << ss.str() << '\n';
    }
    ss.str(std::string());
    display = false;
    for (const auto& quant : puq::UnitSystem::Data->QuantityList) {
      if (puq::BaseUnits(quant.second.definition).dimensions() != dim)
	continue;
      ss << (display ? ", " : "") << quant.first;
      display = true;
    }
    if (display) {
      std::cout << '\n';
      std::cout << "Quantities:  " << ss.str() << '\n';
    }
    std::cout << '\n';
    std::cout << "Dimensions:" << '\n'
	      << '\n';
    puq::DataTable tab({{"Base", 6}, {"Num*Mag", 25}, {"Numerical", 25}, {"Physical", 25}});
    tab.append({"MGS", dim_m.to_string(puq::Format::Display::RESULT), dim.to_string(puq::Format::Display::RESULT), dim.to_string(puq::Format::Display::UNITS)});
    tab.append({"MKS",
	dim_m.to_string({puq::Format::Display::RESULT, puq::Format::Base::MKS}),
	dim.to_string({puq::Format::Display::RESULT, puq::Format::Base::MKS}),
	dim.to_string({puq::Format::Display::UNITS, puq::Format::Base::MKS})});
    tab.append({"CGS",
	dim_m.to_string({puq::Format::Display::RESULT, puq::Format::Base::CGS}),
	dim.to_string({puq::Format::Display::RESULT, puq::Format::Base::CGS}),
	dim.to_string({puq::Format::Display::UNITS, puq::Format::Base::CGS})});
    std::cout << tab.to_string();
    std::cout << '\n';
    if (bus.size() > 0) {
      std::cout << "Base units:" << '\n'
		<< '\n';
      puq::DataTable tab({{"Prefix", 8}, {"Symbol", 8}, {"Exponent", 10}, {"Name", 30}, {"Definition", 30}, {"Dimensions MGS", 22}, {"Allowed prefixes", 22}});
      for (const auto& unit : puq::UnitSystem::Data->UnitList) {
	for (const auto& bu : bus) {
	  if (bu.unit != unit.first)
	    continue;
	  puq::BaseUnits bu_unit({bu});
	  tab.append({bu.prefix,
	      bu.unit,
	      ((puq::to_string(bu.exponent) == "") ? "1" : puq::to_string(bu.exponent)),
	      unit.second.name,
	      unit.second.definition,
	      bu_unit.dimensions().to_string(),
	      puq::to_string(unit.second.use_prefixes, unit.second.allowed_prefixes)});
	}
      }
      for (const auto& quant : puq::UnitSystem::Data->QuantityList) {
	for (const auto& bu : bus) {
	  if (bu.unit != std::string(puq::Symbols::quantity_start) + quant.first + std::string(puq::Symbols::quantity_end))
	    continue;
	  puq::BaseUnits bu_unit({bu});
	  tab.append({bu.prefix,
	      bu.unit,
	      ((puq::to_string(bu.exponent) == "") ? "1" : puq::to_string(bu.exponent)),
	      puq::QuantityNames.at(quant.first),
	      quant.second.definition,
	      bu_unit.dimensions().to_string(),
	      ""});
	}
      }
      std::cout << tab.to_string();
    }
    std::cout << '\n';
  }
    
}
