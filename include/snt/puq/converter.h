#ifndef PUQ_CONVERTER_H
#define PUQ_CONVERTER_H

#include <bitset>
#include <memory>
#include <snt/puq/base_units.h>
#include <snt/puq/dimensions.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/measurement.h>
#include <snt/puq/systems/unit_system.h>
#include <snt/puq/util/data_table.h>
#include <sstream>

namespace snt::puq {

    class Converter {
      private:
        BaseUnits baseunits1;
        BaseUnits baseunits2;
        Result _convert_linear(const Result& m1, const Result& m2);
        Result _convert_logarithmic(const Result& m);
        Result _convert_temperature(Result m);

      public:
        Utype utype;
        Dimensions dimensions1;
        Dimensions dimensions2;
        Converter() : utype(Utype::NUL) {};
        Converter(const BaseUnits& bu1, const BaseUnits& bu2);
        Converter(const std::string& s1, const std::string& s2) : Converter(BaseUnits(s1), BaseUnits(s2)) {};
        Result convert(const Result& m1, const Result& m2 = 1);
    };

    class ConverterException : public puq::Exception {
      private:
        std::string message;

      public:
        ConverterException(const BaseUnits& bu1, const BaseUnits& bu2, const std::string& file, size_t line)
            : ConverterException(bu1, UnitSystem::current.type, bu2, UnitSystem::current.type, file, line) {};
        ConverterException(
            const BaseUnits& bu1,
            const SystemType& s1,
            const BaseUnits& bu2,
            const SystemType& s2,
            const std::string& file,
            const size_t line
        )
            : puq::Exception(prepare_info(bu1, s1, bu2, s2, file, line)) {};

        core::ExceptionInfo prepare_info(
            const BaseUnits& bu1,
            const SystemType& s1,
            const BaseUnits& bu2,
            const SystemType& s2,
            const std::string& file,
            const size_t line
        ) {

            UnitSystem us(s1);
            Dimensions dim1 = bu1.dimensions();
            us.change(s2);
            Dimensions dim2 = bu2.dimensions();
            std::stringstream details;
            details << "Dimensions are incompatible for the requested conversion:" << '\n';
            DataTable tab({{"", 8}, {"System", 10}, {"Unit", 26}, {"Dimensions", 26}});
            us.change(s1);
            tab.append(
                {"From",
                 SystemMap[s1]->SystemAbbrev,
                 bu1.to_string(),
                 ((dim1.to_string(Format::Display::UNITS) == "") ? "1" : dim1.to_string(Format::Display::UNITS))}
            );
            us.change(s2);
            tab.append(
                {"To",
                 SystemMap[s2]->SystemAbbrev,
                 bu2.to_string(),
                 ((dim2.to_string(Format::Display::UNITS) == "") ? "1" : dim2.to_string(Format::Display::UNITS))}
            );
            details << tab.to_string();
            us.change(s1);
            std::stringstream suggestion;
            suggestion << "The following compatible conversions are available:" << '\n';
            tab = DataTable({{"System", 10}, {"Units", 26}, {"Name", 26}, {"Context", 10}});
            std::string mgs = dim1.to_string({Format::Display::UNITS});
            std::string mks = dim1.to_string({Format::Display::UNITS, Format::Base::MKS});
            std::string cgs = dim1.to_string({Format::Display::UNITS, Format::Base::CGS});
            if (mgs == "") {
                suggestion << "1";
            } else {
                tab.append({"BASE", mgs, "MGS base units"});
                if (mgs != mks) {
                    tab.append({"BASE", mks, "MKS base units"});
                }
                if (mgs != cgs && mks != cgs) {
                    tab.append({"BASE", cgs, "CGS base units"});
                }
            }
            // list units with the same dimensions from the first unit system
            us.change(s1);
            std::cout << "current dmap " << UnitSystem::current.data->DimensionMap.size() << std::endl;
            for (auto unit : UnitSystem::current.data->DimensionMap) {
                if (Dimensions(1, unit.second.dimensions) != dim1)
                    continue;
                if (unit.first == mgs || unit.first == mks || unit.first == cgs)
                    continue;
                if (unit.first[0] == Symbols::quantity_start[0])
                    continue;
                tab.append(
                    {SystemMap[s1]->SystemAbbrev,
                     unit.first,
                     UnitSystem::current.data->UnitList.find(unit.first)->second.name}
                );
            }
            for (auto unit : UnitSystem::current.custom->DimensionMap) {
                if (Dimensions(1, unit.second.dimensions) != dim1)
                    continue;
                if (unit.first == mgs || unit.first == mks || unit.first == cgs)
                    continue;
                if (unit.first[0] == Symbols::quantity_start[0])
                    continue;
                tab.append(
                    {SystemMap[s1]->SystemAbbrev,
                     unit.first,
                     UnitSystem::current.custom->UnitList.find(unit.first)->second.name}
                );
            }
            // if the two unit systems are different, list additional conversions
            if (s1 != s2) {
                // list units with the same dimensions from the second unit system
                us.change(s2);
                for (auto unit : UnitSystem::current.data->DimensionMap) {
                    if (Dimensions(1, unit.second.dimensions) != dim1)
                        continue;
                    if (unit.first == mgs || unit.first == mks || unit.first == cgs)
                        continue;
                    if (unit.first[0] == Symbols::quantity_start[0])
                        continue;
                    tab.append(
                        {SystemMap[s2]->SystemAbbrev,
                         unit.first,
                         UnitSystem::current.data->UnitList.find(unit.first)->second.name}
                    );
                }
                for (auto unit : UnitSystem::current.custom->DimensionMap) {
                    if (Dimensions(1, unit.second.dimensions) != dim1)
                        continue;
                    if (unit.first == mgs || unit.first == mks || unit.first == cgs)
                        continue;
                    if (unit.first[0] == Symbols::quantity_start[0])
                        continue;
                    tab.append(
                        {SystemMap[s2]->SystemAbbrev,
                         unit.first,
                         UnitSystem::current.custom->UnitList.find(unit.first)->second.name}
                    );
                }
                us.change(s1);
                for (auto quant : UnitSystem::current.data->QuantityList) {
                    Measurement uv(
                        std::string(Symbols::quantity_start) + quant.first + std::string(Symbols::quantity_end)
                    );
                    Dimensions dim_q = uv.baseunits.dimensions();
                    if (dim_q == dim1) {
                        us.change(s2);
                        uv = Measurement(
                            std::string(Symbols::quantity_start) + quant.first + std::string(Symbols::quantity_end)
                        );
                        dim_q = uv.baseunits.dimensions();
                        for (auto unit : UnitSystem::current.data->DimensionMap) {
                            if (Dimensions(1, unit.second.dimensions) != dim_q)
                                continue;
                            if (unit.first == mgs || unit.first == mks || unit.first == cgs)
                                continue;
                            if (unit.first[0] == Symbols::quantity_start[0])
                                continue;
                            UnitStruct uinfo = UnitSystem::current.data->UnitList[unit.first];
                            if ((uinfo.utype & Utype::LOG) == Utype::LOG)
                                continue;
                            if ((uinfo.utype & Utype::TMP) == Utype::TMP)
                                continue;
                            tab.append(
                                {SystemMap[s2]->SystemAbbrev,
                                 unit.first,
                                 UnitSystem::current.data->UnitList.find(unit.first)->second.name,
                                 quant.first}
                            );
                        }
                        for (auto unit : UnitSystem::current.custom->DimensionMap) {
                            if (Dimensions(1, unit.second.dimensions) != dim_q)
                                continue;
                            if (unit.first == mgs || unit.first == mks || unit.first == cgs)
                                continue;
                            if (unit.first[0] == Symbols::quantity_start[0])
                                continue;
                            UnitStruct uinfo = UnitSystem::current.custom->UnitList[unit.first];
                            if ((uinfo.utype & Utype::LOG) == Utype::LOG)
                                continue;
                            if ((uinfo.utype & Utype::TMP) == Utype::TMP)
                                continue;
                            tab.append(
                                {SystemMap[s2]->SystemAbbrev,
                                 unit.first,
                                 UnitSystem::current.custom->UnitList.find(unit.first)->second.name,
                                 quant.first}
                            );
                        }
                        us.change(s1);
                    }
                }
            }
            suggestion << tab.to_string();

            return core::ExceptionInfo{
                "Incompatible dimensions", details.str(), suggestion.str(), core::SourceLocation{std::move(file), line}
            };
        }
    };

} // namespace snt::puq

#endif // PUQ_CONVERTER_H
