#ifndef MAT_ELEMENT_H
#define MAT_ELEMENT_H

#include "periodic_table.h"
#include "component.h"
#include "matter.h"

namespace snt::mat {

  /**
   * @class Element
   * @brief An element is a pure substance that consists entirely of one type of atom
   */
  class Element: public Component<int>, public Matter {
  public:
    bool natural;                  ///< true if element is a natural isotope
    std::string element;           ///< element symbol
    unsigned int isotope;          ///< isotope type
    unsigned int ionisation;       ///< ionisation state
    double protons;                ///< proton number Z
    double neutrons;               ///< neutron number N
    double electrons;              ///< electron number e

    /**
     * @brief Element class constructor
     *
     * @param expr Component string representation
     * @param prop Component proportion in a Composite
     * @param nat Indicate if element is a natural isotope
     */
    Element(const std::string& expr, int prop = 1, bool nat = true);

  private:
    /**
     * @brief Set parameters of an isotope element
     *
     * @param isodata Isotope data from the periodic table
     * @param ion Ionisation state
     */
    void set_element(const Isotope* isodata, const int ion);
   
    /**
     * @brief Set parameters of an isotope element
     *
     * @param elem Symbol of an element
     * @param iso Isotope number
     * @param ion Ionisation state
     */
    void set_isotope(const std::string& elem, const int iso, const int ion);

    /**
     * @brief set parameters of an element with natural isotopic average
     *
     * @param elem Symbol of an element
     * @param iso Isotope number
     * @param ion Ionisation state
     */
    void set_natural(const std::string& elem, const int iso, const int ion);

    /**
     * @brief set parameters of the most abundant element
     *
     * @param elem Symbol of an element
     * @param iso Isotope number
     * @param ion Ionisation state
     */
    void set_abundant(const std::string& elem, const int iso, const int ion);

  public:
    /**
     * @brief String representation of an element instance
     *
     * @return String representation
     */
    std::string to_string();
  };

} // namespace snt::mat

#endif // MAT_ELEMENT_H
