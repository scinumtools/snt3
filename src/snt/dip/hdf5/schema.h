#ifndef SNT_DIP_HDF5_SCHEMA_H
#define SNT_DIP_HDF5_SCHEMA_H

#include <cstdint>
#include <string_view>

namespace snt::dip::hdf5::schema {

    inline constexpr std::string_view FORMAT = "SciNumTools3 Environment";
    // DIPH5 2.6 also retains applied modifications and evaluated table identity.
    inline constexpr uint64_t VERSION = 2;
    inline constexpr uint64_t VERSION_MINOR = 6;
    inline constexpr uint64_t FIRST_SUPPORTED_VERSION = 1;

    inline constexpr std::string_view ATTR_FORMAT = "_DIPL_Format";
    inline constexpr std::string_view ATTR_VERSION = "_DIPL_Schema_Version";
    inline constexpr std::string_view ATTR_VERSION_MINOR = "_DIPL_Schema_Version_Minor";
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
    inline constexpr std::string_view ATTR_NODE_SCHEMA_ID = "_DIPL_Node_Schema_Id";
    inline constexpr std::string_view ATTR_TABLE_PATH = "_DIPL_Table_Path";
    inline constexpr std::string_view ATTR_TABLE_COLUMN_INDEX = "_DIPL_Table_Column_Index";
    inline constexpr std::string_view ATTR_OVERRIDE = "_DIPL_Override";
    inline constexpr std::string_view ATTR_OVERRIDE_SOURCE = "_DIPL_Override_Source";
    inline constexpr std::string_view ATTR_OVERRIDE_LINE = "_DIPL_Override_Line";
    inline constexpr std::string_view ATTR_OVERRIDE_CODE = "_DIPL_Override_Code";
    inline constexpr std::string_view ATTR_MODIFICATION_SOURCES = "_DIPL_Modification_Sources";
    inline constexpr std::string_view ATTR_MODIFICATION_LINES = "_DIPL_Modification_Lines";
    inline constexpr std::string_view ATTR_MODIFICATION_CODES = "_DIPL_Modification_Codes";
    inline constexpr std::string_view ATTR_SOURCE = "_DIPL_Source";
    inline constexpr std::string_view ATTR_SOURCE_LINE = "_DIPL_Source_Line";
    inline constexpr std::string_view ATTR_SOURCE_CODE = "_DIPL_Source_Code";
    inline constexpr std::string_view GROUP_SOURCES = "_DIPL_Sources";
    inline constexpr std::string_view ATTR_SOURCE_NAME = "_DIPL_Source_Name";
    inline constexpr std::string_view ATTR_SOURCE_PATH = "_DIPL_Source_Path";
    inline constexpr std::string_view ATTR_SOURCE_PARENT = "_DIPL_Source_Parent";
    inline constexpr std::string_view ATTR_SOURCE_PARENT_LINE = "_DIPL_Source_Parent_Line";
    inline constexpr std::string_view ATTR_SOURCE_HASH_ALGORITHM = "_DIPL_Source_Hash_Algorithm";
    inline constexpr std::string_view ATTR_SOURCE_HASH = "_DIPL_Source_Hash";
    inline constexpr std::string_view GROUP_TRACE = "_DIPL_Trace";
    inline constexpr std::string_view GROUP_UNITS = "_DIPL_Units";
    inline constexpr std::string_view ATTR_UNIT_NAME = "_DIPL_Unit_Name";
    inline constexpr std::string_view ATTR_UNIT_DEFINITION = "_DIPL_Unit_Definition";
    inline constexpr std::string_view ATTR_UNIT_ORDER = "_DIPL_Unit_Order";
    inline constexpr std::string_view ATTR_TRACE_ID = "_DIPL_Trace_Id";
    inline constexpr std::string_view ATTR_TRACE_NAME = "_DIPL_Trace_Name";
    inline constexpr std::string_view ATTR_TRACE_KIND = "_DIPL_Trace_Kind";
    inline constexpr std::string_view ATTR_SCHEMA_SOURCE = "_DIPL_Schema_Source";
    inline constexpr std::string_view ATTR_SCHEMA_SOURCE_LINE = "_DIPL_Schema_Source_Line";
    inline constexpr std::string_view KIND_VALUE_GROUP = "value_group";
    inline constexpr std::string_view VALUE_PAYLOAD = "_DIPL_Value";

} // namespace snt::dip::hdf5::schema

#endif
