#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_functions.h>
#include <stdexcept>
#include <string>

namespace snt::dip {

    void FunctionList::append_value(const std::string& name, ValueFunctionType func) {
        auto result = value_functions.insert({name, func});
        if (!result.second)
            throw dip::EnvironmentException(
                "Duplicate value function",
                "The value function name must be unique within the environment function list.",
                "A value function named `" + name + "` already exists in the environment function list.",
                "Choose a different value function name.",
                __FILE__,
                __LINE__
            );
    }

    void FunctionList::append_table(const std::string& name, TableFunctionType func) {
        auto result = table_functions.insert({name, func});
        if (!result.second)
            throw dip::EnvironmentException(
                "Duplicate table function",
                "The table function name must be unique within the environment function list.",
                "A table function named `" + name + "` already exists in the environment function list.",
                "Choose a different table function name.",
                __FILE__,
                __LINE__
            );
    }

    FunctionList::ValueFunctionType FunctionList::get_value(const std::string& name) const {
        auto it = value_functions.find(name);
        if (it == value_functions.end())
            throw dip::EnvironmentException(
                "Unknown value function",
                "The requested value function must exist in the environment function list.",
                "The value function `" + name + "` was not found in the environment function list.",
                "Check whether the value function name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

    FunctionList::TableFunctionType FunctionList::get_table(const std::string& name) const {
        auto it = table_functions.find(name);
        if (it == table_functions.end())
            throw dip::EnvironmentException(
                "Unknown table function",
                "The requested table function must exist in the environment function list.",
                "The table function `" + name + "` was not found in the environment function list.",
                "Check whether the table function name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

} // namespace snt::dip
