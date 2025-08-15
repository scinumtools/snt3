#include "exs/exs.h"
#include "settings.h"

typedef std::unique_ptr<bool> UniquePtrType;

class LogicalAtom : public exs::AtomBase<UniquePtrType> {
public:
  // Constructor from unique_ptr
  LogicalAtom(UniquePtrType b) : AtomBase(std::move(b)) {};
  // Deep copy constructor
  LogicalAtom(const LogicalAtom& a);
  LogicalAtom& operator=(const LogicalAtom& a);
  // Move constructor
  LogicalAtom(LogicalAtom&& a) noexcept = default;
  LogicalAtom& operator=(LogicalAtom&& a) noexcept = default;

  static UniquePtrType from_string(std::string& s);
  std::string to_string();
  void logical_and(LogicalAtom* other);
};
