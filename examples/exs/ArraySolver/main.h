#include <snt/exs/solver.h>
#include <vector>

using namespace snt;

enum CustomOperatorType { ARRAY_OPERATOR = exs::NUM_OPERATOR_TYPES };

using AtomValueType = std::vector<double>;

class CustomAtom final : public exs::AtomBase<CustomAtom, AtomValueType> {
  public:
    CustomAtom(const CustomAtom& a) = default;
    CustomAtom(AtomValueType v) : AtomBase(std::move(v)) {};
    static AtomValueType from_string(const std::string& s, exs::BaseSettings* set = nullptr) { return {std::stod(s)}; }
    std::string to_string() override;
    void math_add(CustomAtom* other) override;
    void math_subtract(CustomAtom* other) override;
    void math_negate() override;
};

class OperatorArray : public exs::OperatorGroup<0> {
  public:
    OperatorArray() : OperatorGroup<0>("arr", {"", "[", "]", ","}, ARRAY_OPERATOR) {};
    void operate_group(exs::TokenListBase* tokens) override;
};
