#ifndef DIP_LIST_UNIT_H
#define DIP_LIST_UNIT_H

#include <map>
#include <string>

namespace snt::dip {

  struct EnvUnit {
    std::string name;       // unit key
    std::string definition; // unit definition
  };

  class UnitList {
  private:
    std::map<std::string, EnvUnit> units;

  public:
    UnitList();
    void append(const std::string& name, const std::string& definition);
    void append(const std::string& name, const EnvUnit& src);
    EnvUnit& at(const std::string& name);
    const EnvUnit& at(const std::string& name) const;
  };

} // namespace snt::dip

#endif // DIP_LIST_UNIT_H
