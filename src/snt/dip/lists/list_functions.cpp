#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_functions.h>
#include <stdexcept>
#include <string>
#include <utility>

namespace snt::dip {

    namespace {

        std::string next_function_id(std::map<std::string, size_t>& counters, const std::string& parent_id) {
            const std::string scope = parent_id.empty() ? "ENV" : parent_id;
            return scope + "_FUNCTION" + std::to_string(counters[scope]++);
        }

    } // namespace

    void FunctionList::append_value(const std::string& name, DataFunctionType func, const std::string& parent_id) {
        if (data_functions.find(name) != data_functions.end())
            throw dip::EnvironmentException(
                "Duplicate value function",
                "A value function named `" + name + "` already exists in the environment function list.",
                "Choose a different value function name.",
                __FILE__,
                __LINE__
            );
        data_functions.insert({name, std::move(func)});
        data_function_info.insert({name, {next_function_id(id_counters, parent_id), name, FunctionKind::Value}});
    }

    void FunctionList::append_nodes(const std::string& name, NodesFunctionType func, const std::string& parent_id) {
        if (nodes_functions.find(name) != nodes_functions.end())
            throw dip::EnvironmentException(
                "Duplicate table function",
                "A table function named `" + name + "` already exists in the environment function list.",
                "Choose a different table function name.",
                __FILE__,
                __LINE__
            );
        nodes_functions.insert({name, std::move(func)});
        nodes_function_info.insert({name, {next_function_id(id_counters, parent_id), name, FunctionKind::Nodes}});
    }

    FunctionList::DataFunctionType FunctionList::get_value(const std::string& name) const {
        auto it = data_functions.find(name);
        if (it == data_functions.end())
            throw dip::EnvironmentException(
                "Unknown value function",
                "The value function `" + name + "` was not found in the environment function list.",
                "Check whether the value function name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

    FunctionList::NodesFunctionType FunctionList::get_nodes(const std::string& name) const {
        auto it = nodes_functions.find(name);
        if (it == nodes_functions.end())
            throw dip::EnvironmentException(
                "Unknown table function",
                "The table function `" + name + "` was not found in the environment function list.",
                "Check whether the table function name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

    const EnvFunction& FunctionList::get_value_info(const std::string& name) const {
        auto it = data_functions.find(name);
        if (it == data_functions.end())
            throw dip::EnvironmentException(
                "Unknown value function",
                "The value function `" + name + "` was not found in the environment function list.",
                "Check whether the value function name is correct.",
                __FILE__,
                __LINE__
            );
        return data_function_info.at(name);
    }

    const EnvFunction& FunctionList::get_nodes_info(const std::string& name) const {
        auto it = nodes_functions.find(name);
        if (it == nodes_functions.end())
            throw dip::EnvironmentException(
                "Unknown table function",
                "The table function `" + name + "` was not found in the environment function list.",
                "Check whether the table function name is correct.",
                __FILE__,
                __LINE__
            );
        return nodes_function_info.at(name);
    }

    std::vector<EnvFunction> FunctionList::entries() const {
        std::vector<EnvFunction> result;
        result.reserve(data_function_info.size() + nodes_function_info.size());
        for (const auto& entry : data_function_info)
            result.push_back(entry.second);
        for (const auto& entry : nodes_function_info)
            result.push_back(entry.second);
        return result;
    }

} // namespace snt::dip
