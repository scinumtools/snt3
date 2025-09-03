#include "../environment.h"
#include "list_source.h"

#include <stdexcept>

namespace snt::dip {

  SourceList::SourceList() = default;

  void SourceList::append(const std::string& name, const std::string& path, const std::string& code,
                          const Source& parent) {
    if (sources.contains(name))
      throw std::invalid_argument("Source with the same name already exists: " + name);
    sources.insert({name, {name, path, code, parent}});
  }

  void SourceList::append(const std::string& name, const EnvSource& src) {
    if (sources.contains(name))
      throw std::invalid_argument("Source with the same name already exists: " + name);
    sources.insert({name, src});
  }

  EnvSource& SourceList::at(const std::string& name) {
    auto it = sources.find(name);
    if (it == sources.end())
      throw std::runtime_error("Following source was not found in the environment source list: " +
                               name);
    else
      return it->second;
  }

  const EnvSource& SourceList::at(const std::string& name) const {
    auto it = sources.find(name);
    if (it == sources.end())
      throw std::runtime_error("Following source was not found in the environment source list: " +
                               name);
    else
      return it->second;
  }

} // namespace snt::dip
