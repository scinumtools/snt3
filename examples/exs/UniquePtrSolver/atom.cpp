#import "main.h"
#import "snt/settings.h"

LogicalAtom::LogicalAtom(const LogicalAtom& a) : AtomBase(std::make_unique<bool>(*a.value)) {};

LogicalAtom& LogicalAtom::operator=(const LogicalAtom& a) {
  if (this != &a) {
    value = std::make_unique<bool>(*a.value);
  }
  return *this;
}

UniquePtrType LogicalAtom::from_string(std::string& s, exs::BaseSettings* set) {
  if (s == snt::KEYWORD_TRUE)
    return std::make_unique<bool>(true);
  else if (s == snt::KEYWORD_FALSE)
    return std::make_unique<bool>(false);
  else
    throw std::runtime_error("Invalid atom value: " + s);
}

std::string LogicalAtom::to_string() {
  return std::string((*value) ? snt::KEYWORD_TRUE : snt::KEYWORD_FALSE);
}

void LogicalAtom::logical_and(LogicalAtom* other) {
  value = std::make_unique<bool>(*value && *(other->value));
}
