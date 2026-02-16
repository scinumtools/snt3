#include <snt/dip/lists/list_functions.h>

#include <stdexcept>
#include <string>

namespace snt::dip {

  void FunctionList::append_value(const std::string& name, ValueFunctionType func) {
    auto result = value_functions.insert({name, func});
    if (!result.second)
      throw std::runtime_error("Value function with the following name already exists: " + name);
  }

  void FunctionList::append_table(const std::string& name, TableFunctionType func) {
    auto result = table_functions.insert({name, func});
    if (!result.second)
      throw std::runtime_error("Table function with the following name already exists: " + name);
  }

  FunctionList::ValueFunctionType FunctionList::get_value(const std::string& name) const {
    auto it = value_functions.find(name);
    if (it == value_functions.end())
      throw std::runtime_error("Value function with the following name does not exists: " + name);
    else
      return it->second;
  }

  FunctionList::TableFunctionType FunctionList::get_table(const std::string& name) const {
    auto it = table_functions.find(name);
    if (it == table_functions.end())
      throw std::runtime_error("Table function with the following name does not exists: " + name);
    else
      return it->second;
  }

} // namespace snt::dip
