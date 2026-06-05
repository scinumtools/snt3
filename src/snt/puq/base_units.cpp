#include <fstream>
#include <iostream>
#include <snt/puq/base_units.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/math/pow.h>
#include <snt/puq/settings.h>
#include <snt/puq/solver/unit_solver.h>
#include <snt/puq/to_string.h>
#include <sstream>
#include <string>

namespace snt::puq {

    BaseUnits::BaseUnits(const std::string& s) {
        UnitSolver solver;
        UnitAtom atom = solver.solve(s);
        baseunits = atom.value.baseunits.baseunits;
    }

    std::string BaseUnit::to_string(const UnitFormat& format) {
        std::stringstream ss;
        ss << prefix << unit;
        if (exponent_to_float(exponent) != 1)
            ss << puq::to_string(exponent, format);
        return ss.str();
    }

    void BaseUnits::append(const BaseUnit& bu) {
        bool exists = false;
        for (auto it = baseunits.begin(); it != baseunits.end(); ++it) {
            if (it->prefix == bu.prefix && it->unit == bu.unit) {
                exists = true;
                it->exponent = std::visit(
                    [](auto const& a, auto const& b) -> ExponentVariant { return a + b; }, it->exponent, bu.exponent
                );
                // removing zero exponents
                if (exponent_to_float(it->exponent) == 0) {
                    baseunits.erase(it);
                }
                break;
            }
        }
        if (!exists && exponent_to_float(bu.exponent) != 0) {
            baseunits.push_back(bu);
        }
    }

    void BaseUnits::append(const std::string& p, const std::string& u, ExponentVariant e) {
        append(BaseUnit(p, u, e));
    }

    void BaseUnits::append(const std::string& p, const std::string& u, int n, int d) {
        append(BaseUnit(p, u, n, d));
    }

    std::string BaseUnits::to_string(const UnitFormat& format) const {
        std::stringstream ss;
        std::string multiply = format.multiply_symbol();
        for (auto unit : baseunits) {
            ss << unit.to_string(format) << multiply;
        }
        std::string s = ss.str();
        return s.substr(0, s.size() - multiply.size());
    }

    const BaseUnit& BaseUnits::operator[](int index) const {
        return baseunits[index];
    }

    void BaseUnits::operator*=(const BaseUnits& bu) {
        for (const auto& unit : bu) {
            append(unit);
        }
    }
    BaseUnits operator*(BaseUnits bu1, const BaseUnits& bu2) {
        bu1 *= bu2;
        return bu1;
    }

    void BaseUnits::operator/=(const BaseUnits& bu) {
        for (auto unit : bu) {
            unit.exponent = std::visit([](auto const& v) -> ExponentVariant { return -v; }, unit.exponent);
            append(unit);
        }
    }
    BaseUnits operator/(BaseUnits bu1, const BaseUnits& bu2) {
        bu1 /= bu2;
        return bu1;
    }

    // void BaseUnits::operator*=(const ExponentVariant& exp) {
    //   for (auto& unit : baseunits) {
    //     unit.exponent = std::visit([](auto const& a, auto const& b) -> ExponentVariant {
    //       return a * b;
    //     }, unit.exponent, exp);
    //   }
    // }
    // BaseUnits operator*(BaseUnits bu, const ExponentVariant& exp) {
    //   bu *= exp;
    //   return bu;
    // }
    //
    // void BaseUnits::operator/=(const ExponentVariant& exp) {
    //   for (auto& unit : baseunits) {
    //     unit.exponent = std::visit([](auto const& a, auto const& b) -> ExponentVariant {
    //       return a / b;
    //     }, unit.exponent, exp);
    //   }
    // }
    // BaseUnits operator/(BaseUnits bu, const ExponentVariant& exp) {
    //   bu /= exp;
    //   return bu;
    // }

    std::ostream& operator<<(std::ostream& os, const BaseUnits& bu) {
        os << bu.to_string();
        return os;
    }

    BaseUnits::ListType::const_iterator BaseUnits::begin() const {
        return baseunits.begin();
    }

    BaseUnits::ListType::const_iterator BaseUnits::end() const {
        return baseunits.end();
    }

    std::size_t BaseUnits::size() const {
        return baseunits.size();
    }

    bool BaseUnits::has_dimensions() const {
        if (baseunits.empty())
            return false;
        Dimensions dims = dimensions();
        return dims.has_dimensions();
    }

    Dimensions BaseUnits::dimensions() const {
        Dimensions dim;
        dim.symbols.reserve(baseunits.size());
        for (auto& bu : baseunits) {
            // find prefix
            auto prefix = UnitPrefixList.find(bu.prefix);
            if (prefix != UnitPrefixList.end()) {
                dim.numerical *= math::pow(prefix->second.result, exponent_to_float(bu.exponent));
            }
            // quantity
            if (bu.unit.rfind(Symbols::quantity_start, 0) == 0 || bu.unit.rfind(Symbols::si_factor_start, 0) == 0) {
                auto dmap_data = UnitSystem::current.data->DimensionMap.find(bu.unit);
                auto dmap_custom = UnitSystem::current.custom->DimensionMap.find(bu.unit);
                if (dmap_data != UnitSystem::current.data->DimensionMap.end()) {
                    dim.utype = dim.utype | Utype::LIN;
                    dim.symbols.push_back(dmap_data->first);
                    Result result(dmap_data->second.estimate, dmap_data->second.uncertainty);
                    dim.numerical *= math::pow(result, exponent_to_float(bu.exponent));
                    for (int i = 0; i < Config::num_basedim; i++) {
                        dim.physical[i] =
                            add_exp(dim.physical[i], mul_exp(dmap_data->second.dimensions[i], bu.exponent));
                    }
                    continue;
                } else if (dmap_custom != UnitSystem::current.custom->DimensionMap.end()) {
                    dim.utype = dim.utype | Utype::LIN;
                    dim.symbols.push_back(dmap_custom->first);
                    Result result(dmap_custom->second.estimate, dmap_custom->second.uncertainty);
                    dim.numerical *= math::pow(result, exponent_to_float(bu.exponent));
                    for (int i = 0; i < Config::num_basedim; i++) {
                        dim.physical[i] =
                            add_exp(dim.physical[i], mul_exp(dmap_custom->second.dimensions[i], bu.exponent));
                    }
                    continue;
                } else {
                    throw MeasurementExcept("Undefined quantity symbol: " + bu.unit);
                }
            }
            // dimensions
            std::string unit_name;
            Utype unit_utype = Utype::NUL;
            auto unit_data = UnitSystem::current.data->UnitList.find(bu.unit);
            auto unit_custom = UnitSystem::current.custom->UnitList.find(bu.unit);
            if (unit_data != UnitSystem::current.data->UnitList.end()) {
                unit_name = unit_data->first;
                unit_utype = unit_data->second.utype;
            } else if (unit_custom != UnitSystem::current.custom->UnitList.end()) {
                unit_name = unit_custom->first;
                unit_utype = unit_custom->second.utype;
            }
            if (unit_utype != Utype::NUL) {
                if (unit_name == bu.unit) {
                    if ((unit_utype & Utype::LIN) == Utype::LIN) // standard linear conversion
                        dim.utype = dim.utype | Utype::LIN;
                    if (unit_utype == Utype::TMP) // unit requires conversion of temperatures
                        dim.utype = dim.utype | unit_utype;
                    if (unit_utype == Utype::LOG) // unit requires logarithmic conversions
                        dim.utype = dim.utype | unit_utype;
                    dim.symbols.push_back(unit_name);
                    auto dmap_data = UnitSystem::current.data->DimensionMap.find(unit_name);
                    auto dmap_custom = UnitSystem::current.custom->DimensionMap.find(unit_name);
                    if (dmap_data != UnitSystem::current.data->DimensionMap.end()) {
                        Result result(dmap_data->second.estimate, dmap_data->second.uncertainty);
                        dim.numerical *= math::pow(result, exponent_to_float(bu.exponent));
                        for (int i = 0; i < Config::num_basedim; i++) {
                            dim.physical[i] =
                                add_exp(dim.physical[i], mul_exp(dmap_data->second.dimensions[i], bu.exponent));
                        }
                    } else if (dmap_custom != UnitSystem::current.custom->DimensionMap.end()) {
                        Result result(dmap_custom->second.estimate, dmap_custom->second.uncertainty);
                        dim.numerical *= math::pow(result, exponent_to_float(bu.exponent));
                        for (int i = 0; i < Config::num_basedim; i++) {
                            dim.physical[i] =
                                add_exp(dim.physical[i], mul_exp(dmap_custom->second.dimensions[i], bu.exponent));
                        }
                    } else {
                        throw MeasurementExcept("Undefined unit symbol: " + bu.unit);
                    }
                }
            } else {
                throw MeasurementExcept("Undefined unit symbol: " + bu.unit);
            }
        }
        if ((dim.utype & Utype::NUL) == Utype::NUL) // add liear utype for dimensionless values
            dim.utype = dim.utype | Utype::LIN;
        // std::cout << "A " << std::bitset<8>((int)dim.utype) << std::endl;
        return dim;
    }

} // namespace snt::puq
