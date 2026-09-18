#ifndef SNT_DIP_HDF5_SCHEMA_H
#define SNT_DIP_HDF5_SCHEMA_H

#include <string_view>

namespace snt::dip::hdf5::schema {

    inline constexpr std::string_view FORMAT = "SciNumTools3 Environment";
    inline constexpr uint64_t VERSION = 1;

    inline constexpr std::string_view ATTR_FORMAT = "_DIPL_Format";
    inline constexpr std::string_view ATTR_VERSION = "_DIPL_Schema_Version";
    inline constexpr std::string_view ATTR_KIND = "_DIPL_Kind";
    inline constexpr std::string_view ATTR_KEY = "_DIPL_Key";
    inline constexpr std::string_view ATTR_INDEX = "_DIPL_Index";
    inline constexpr std::string_view ATTR_PATH = "_DIPL_Path";
    inline constexpr std::string_view ATTR_NODE_TYPE = "_DIPL_Node_Type";
    inline constexpr std::string_view ATTR_VALUE_TYPE = "_DIPL_Value_Type";
    inline constexpr std::string_view ATTR_ARRAY = "_DIPL_Array";
    inline constexpr std::string_view ATTR_CONSTANT = "_DIPL_Constant";
    inline constexpr std::string_view ATTR_CONDITION = "_DIPL_Condition";
    inline constexpr std::string_view ATTR_VALUE_ORIGIN = "_DIPL_Value_Origin";
    inline constexpr std::string_view ATTR_FORMAT_SPEC = "_DIPL_Format_Spec";
    inline constexpr std::string_view ATTR_TAGS = "_DIPL_Tags";
    inline constexpr std::string_view ATTR_OPTIONS = "_DIPL_Options";
    inline constexpr std::string_view ATTR_OPTION_UNITS = "_DIPL_Option_Units";
    inline constexpr std::string_view ATTR_SCHEMAS = "_DIPL_Schemas";
    inline constexpr std::string_view ATTR_SOURCE = "_DIPL_Source";
    inline constexpr std::string_view ATTR_SOURCE_LINE = "_DIPL_Source_Line";
    inline constexpr std::string_view ATTR_SOURCE_CODE = "_DIPL_Source_Code";

} // namespace snt::dip::hdf5::schema

#endif
