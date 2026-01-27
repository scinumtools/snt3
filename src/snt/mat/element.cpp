#include "element.h"
#include "periodic_table.h"
#include "../puq.h"

#include <sstream>
#include <regex>

namespace snt::mat {

  Element::Element(const std::string& expr, int prop, bool nat):
    elementProps({0, 0, 0}), Part<int>(prop, expr), natural(nat) {
    if (expr.size()==3 && expr[0]=='[' && expr[2]==']') {
      element=expr;
      isotope=0;
      ionisation=0;
      matterProps.volume = 1;
      if (expr[1]=='p') {
	matterProps.mass = puq::Quantity("{m_p}");
	elementProps = {1, 0, 0};
      } else if (expr[1]=='n') {
	matterProps.mass = puq::Quantity("{m_n}");
	elementProps = {0, 1, 0};			   
      } else if (expr[1]=='e') {
	matterProps.mass = puq::Quantity("{m_e}");
	elementProps = {0, 0, 1};
      } else {
	throw std::runtime_error("Element could not be determined from the expression: "+expr);
      }
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
    matterProps.mass = puq::Quantity(isodata->atomic_number, "u") + puq::Quantity(ion, "{m_e}");
    elementProps.protons = isodata->protons;
    elementProps.neutrons = static_cast<double>(isodata->isotope_number) - static_cast<double>(isodata->protons);
    elementProps.electrons = isodata->protons + ion;
    if (elementProps.electrons<0)
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
    elementProps.neutrons = 0;
    for (const auto& isodata: PT_DATA) {
      if (isodata.symbol==elem && isodata.natural_abundance>0) {
	dmass += isodata.atomic_number * isodata.natural_abundance;
	elementProps.protons = isodata.protons;
	elementProps.neutrons += (static_cast<double>(isodata.isotope_number) - static_cast<double>(isodata.protons))*isodata.natural_abundance;
	elementProps.electrons = isodata.protons + ion;
      }
    }
    if (dmass==0)
	throw std::runtime_error("Element has no natural abundance: "+elem);      
    matterProps.mass = puq::Quantity(dmass, "u") + puq::Quantity(ion, "{m_e}");
  }

  Element Element::from_string(const std::string& expr) {
    return Element(expr);
  }

  std::string Element::to_string() {
    std::stringstream ss;
    if (element[0]=='[') {
      ss << element;
    } else {
      ss << element;
      if (isotope!=0 || ionisation!=0) {
	ss << "{";
	if (isotope!=0)
	  ss << isotope;
	if (ionisation!=0)
	  ss << ionisation;
	ss << "}";
      }
    }
    return ss.str();
  }
  
  void Element::math_add(Element* other) {
  }

  void Element::math_multiply(Element* other) {
  }

}
