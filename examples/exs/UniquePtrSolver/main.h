#include <snt/exs/solver.h>

using namespace snt;

typedef std::unique_ptr<bool> UniquePtrType;

class LogicalAtom final : public exs::AtomBase<LogicalAtom,UniquePtrType> {
public:
  // Constructor from unique_ptr
  LogicalAtom(UniquePtrType b) : AtomBase(std::move(b)) {};
  // Deep copy constructor
  LogicalAtom(const LogicalAtom& a);
  LogicalAtom& operator=(const LogicalAtom& a);
  // Move constructor
  LogicalAtom(LogicalAtom&& a) noexcept = default;
  LogicalAtom& operator=(LogicalAtom&& a) noexcept = default;

  static UniquePtrType from_string(std::string& s, exs::BaseSettings* set = nullptr);
  std::string to_string() override;
  void logical_and(LogicalAtom* other) override;
};
