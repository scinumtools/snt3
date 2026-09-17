#include <snt/exs/solver.h>
#include <variant>

using namespace snt;

enum CustomOperatorType { LENGTH_OPERATOR = exs::NUM_OPERATOR_TYPES };

using AtomValueType = std::variant<std::string, bool, size_t>;

class CustomAtom final : public exs::AtomBase<CustomAtom, AtomValueType> {
  public:
    CustomAtom(const CustomAtom& a) = default;
    CustomAtom(AtomValueType v) : AtomBase(std::move(v)) {};
    static AtomValueType from_string(const std::string& s, exs::BaseSettings* set = nullptr) { return s; }
    std::string to_string() override;
    void comparison_less(CustomAtom* other) override;
    void custom_length();
};

class OperatorLength : public exs::OperatorGroup<1> {
  public:
    OperatorLength();
    void operate_group(exs::TokenListBase* tokens) override;
};
