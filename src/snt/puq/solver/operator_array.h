#ifndef PUQ_OPERATOR_ARRAY_H
#define PUQ_OPERATOR_ARRAY_H

namespace snt::puq {

#if defined(MAGNITUDE_ARRAYS) || defined(MAGNITUDE_VALUES)

  enum CustomOperatorType {
    ARRAY_OPERATOR = exs::NUM_OPERATOR_TYPES
  };

  class OperatorArray : public exs::OperatorGroup<UnitAtom> {
  public:
    OperatorArray() : OperatorGroup<UnitAtom>("arr", {"", std::string(SYMBOL_ARRAY_START), std::string(SYMBOL_ARRAY_END), ","}, ARRAY_OPERATOR) {}
    void operate_group(exs::TokenListBase<UnitAtom>* tokens);
  };

#endif

} // namespace snt::puq

#endif // PUQ_OPERATOR_ARRAY_H
