#ifndef PUQ_EXCEPTIONS_H
#define PUQ_EXCEPTIONS_H

#include <string>

namespace snt::puq {

  class CalculatorExcept : public std::exception {
  private:
    std::string message;

  public:
    CalculatorExcept(const std::string& m) : message(m) {}
    const char* what() const noexcept override {
      return message.c_str();
    }
  };
  
  class UnitValueExcept : public std::exception {
  private:
    std::string message;

  public:
    UnitValueExcept(std::string m) : message(m) {}
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

  class UnitSystemExcept : public std::exception {
  private:
    std::string message;

  public:
    UnitSystemExcept(const std::string m) : message(m) {}
    UnitSystemExcept(const SystemType st1, const SystemType st2) {
      auto it1 = SystemMap.find(st1);
      auto it2 = SystemMap.find(st2);
      if (it1 == SystemMap.end() || it2 == SystemMap.end()) {
        message = "Unknown system type";
      } else {
        std::stringstream ss;
        ss << "Incompatible unit systems: ";
        ss << it1->second->SystemAbbrev;
        ss << " != ";
        ss << it2->second->SystemAbbrev;
        ss << std::endl;
        message = ss.str();
      }
    }
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

} // namespace snt::puq

#endif // PUQ_EXCEPTIONS_H
