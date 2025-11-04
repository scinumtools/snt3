#include "element.h"
#include "periodic_table.h"
#include "../puq.h"

#include <sstream>
#include <regex>

namespace snt::mat {

  Element::Element(const std::string& expr, int prop, bool nat):
    Component<int>(expr, prop), natural(nat) {
    if (expr=="[p]") {
      element=expr, mass = puq::Quantity("{m_p}"), protons=1, neutrons=0, electrons=0, isotope=0, ionisation=0;
    } else if (expr=="[n]") {
      element=expr, mass = puq::Quantity("{m_n}"), protons=0, neutrons=1, electrons=0, isotope=0, ionisation=0; 
    } else if (expr=="[e]") {
      element=expr, mass = puq::Quantity("{m_e}"), protons=0, neutrons=0, electrons=1, isotope=0, ionisation=0;
    } else {
      std::smatch m;
      std::regex pattern("^([a-zA-Z]{1,2})(?:\\{([0-9]+|)([-+][0-9]*|)\\}|)?$");
      if (std::regex_match(expr, m, pattern)) {
	std::string elem;
	int iso, ion;
	if (m[1]=="D") {
	  elem="H"; iso=2;
	} else if (m[1]=="T") {
	  elem="H"; iso=3;
	} else if (m[2]!="") {
	  elem=m[1]; iso=std::stoi(m[2]);
	} else {
	  elem=m[1]; iso=0;
	}
	if (m[3]=="-" || m[3]=="+")
	  ion=std::stoi(std::string(m[3])+"1");
	else if (m[3]!="")
	  ion=std::stoi(m[3]);
	else
	  ion=0;
	if (iso>0)
	  set_isotope(elem,iso,ion);
	else if (natural)
	  set_natural(elem,iso,ion);
	else
	  set_abundant(elem,iso,ion);
      } else {
	throw std::runtime_error("Element could not be determined from the expression: "+expr);
      }
    }
  }

  void Element::set_element(const Isotope* isodata, const int ion) {
    element = isodata->symbol;
    isotope = isodata->isotope_number;
    ionisation = ion;
    mass = puq::Quantity(isodata->atomic_number, "Da") + puq::Quantity(ion, "{m_e}");
    protons = isodata->protons;
    neutrons = static_cast<double>(isodata->isotope_number) - static_cast<double>(isodata->protons);
    electrons = static_cast<double>(isodata->protons) + static_cast<double>(ion);
    if (electrons<0)
      throw std::runtime_error("Number of electrons cannot be negative.");
  }
  
  void Element::set_isotope(const std::string& elem, const int iso, const int ion) {
    if (iso==0)
      throw std::runtime_error("Isotope number cannot be zero");
    const Isotope* isodata = nullptr;
    for (size_t i=0; i<PT_NUM_DATA; i++) {
      if (PT_DATA[i].symbol==elem and PT_DATA[i].isotope_number==iso) {
	isodata = &PT_DATA[i];
      }
    }
    if (isodata == nullptr)
      throw std::runtime_error("Could not finde isotope "+std::to_string(iso)+" of element "+elem);
    set_element(isodata, ion);
  }

  void Element::set_abundant(const std::string& elem, const int iso, const int ion) {
    const Isotope* isodata = nullptr;
    double NA = 0;
    for (size_t i=0; i<PT_NUM_DATA; i++) {
      if (PT_DATA[i].symbol==elem && PT_DATA[i].natural_abundance > NA) {
	isodata = &PT_DATA[i];
	NA = PT_DATA[i].natural_abundance;
      }
    }    
    if (isodata == nullptr)
      throw std::runtime_error("Could not find the most abundant isotope of "+elem);
    set_element(isodata, ion);
  }
  
  void Element::set_natural(const std::string& elem, const int iso, const int ion) {
    element = elem;
    isotope = 0;
    ionisation = ion;
    double dmass = 0;
    neutrons = 0;
    for (const auto& isodata: PT_DATA) {
      if (isodata.symbol==elem && isodata.natural_abundance>0) {
	dmass += isodata.atomic_number*isodata.natural_abundance;
	protons = isodata.protons;
	neutrons += (static_cast<double>(isodata.isotope_number) - static_cast<double>(isodata.protons))*isodata.natural_abundance;
	electrons = static_cast<double>(isodata.protons) + static_cast<double>(ion);
      }
    }
    mass = puq::Quantity(dmass, "Da") + puq::Quantity(ion, "{m_e}");
  }

  std::string Element::to_string() {
    std::stringstream ss;
    if (element[0]=='[') {
      ss << element;
    } else {
      ss << element;
      if (isotope!=0 && ionisation!=0)
	ss << "{";
      if (isotope!=0)
	ss << isotope;
      if (ionisation!=0)
	ss << ionisation;
      if (isotope!=0 && ionisation!=0)
	ss << "}";
    }
    return ss.str();
  }
  
}
