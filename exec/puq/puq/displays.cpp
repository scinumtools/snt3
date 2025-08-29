#include "puq/lists.h"
#include "puq/util/data_table.h"

#include "main.h"

void display_info(const std::string& expr) {
  puq::UnitValue uv = puq::Quantity(expr).value;
  puq::BaseUnits bus = uv.baseunits;
  puq::Dimensions dim = bus.dimensions();
  puq::Dimensions dim_m = bus.dimensions();
  dim_m.numerical *= uv.magnitude;
  std::cout << '\n'
            << "Expression:  " << expr << '\n'
            << '\n';
  std::cout << "Unit system: " << puq::UnitSystem::Data->SystemAbbrev << " (" << puq::UnitSystem::Data->SystemName << ")" << '\n';
  std::cout << "Magnitude:   " << nostd::to_string(uv.magnitude) << '\n';
  std::cout << "Base units:  " << nostd::to_string(uv.baseunits) << '\n';
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
  tab.append({"MGS", dim_m.to_string(puq::Format::Display::MAGNITUDE), dim.to_string(puq::Format::Display::MAGNITUDE), dim.to_string(puq::Format::Display::UNITS)});
  tab.append({"MKS",
              dim_m.to_string({puq::Format::Display::MAGNITUDE, puq::Format::Base::MKS}),
              dim.to_string({puq::Format::Display::MAGNITUDE, puq::Format::Base::MKS}),
              dim.to_string({puq::Format::Display::UNITS, puq::Format::Base::MKS})});
  tab.append({"CGS",
              dim_m.to_string({puq::Format::Display::MAGNITUDE, puq::Format::Base::CGS}),
              dim.to_string({puq::Format::Display::MAGNITUDE, puq::Format::Base::CGS}),
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
                    ((nostd::to_string(bu.exponent) == "") ? "1" : nostd::to_string(bu.exponent)),
                    unit.second.name,
                    unit.second.definition,
                    bu_unit.dimensions().to_string(),
                    nostd::to_string(unit.second.use_prefixes, unit.second.allowed_prefixes)});
      }
    }
    for (const auto& quant : puq::UnitSystem::Data->QuantityList) {
      for (const auto& bu : bus) {
        if (bu.unit != SYMBOL_QUANTITY_START + quant.first + SYMBOL_QUANTITY_END)
          continue;
        puq::BaseUnits bu_unit({bu});
        tab.append({bu.prefix,
                    bu.unit,
                    ((nostd::to_string(bu.exponent) == "") ? "1" : nostd::to_string(bu.exponent)),
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

void display_lists(std::deque<std::string>& convert) {
  std::stringstream ss;
  ss << '\n';
  if (convert.size() > 0)
    ss << "List '" << convert[0] << "' does not exist." << '\n'
       << '\n';
  ss << "Available lists:" << '\n'
     << '\n';
  ss << "prefix  unit prefixes" << '\n';
  ss << "base    base units" << '\n';
  ss << "deriv   derived units" << '\n';
  ss << "log     logarithmic units" << '\n';
  ss << "temp    temperature units" << '\n';
  ss << "const   constants" << '\n';
  ss << "quant   quantities" << '\n';
  ss << "sys     unit systems" << '\n';
  throw std::runtime_error(ss.str());
}
