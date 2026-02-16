#ifndef PUQ_OPERATOR_ARRAY_H
#define PUQ_OPERATOR_ARRAY_H

#include <snt/exs/operator_group.h>
#include <snt/puq/settings.h>
#include <snt/puq/solver/unit_atom.h>

namespace snt::puq {

#if defined(MAGNITUDE_VALUES)

  enum CustomOperatorType {
    ARRAY_OPERATOR = exs::NUM_OPERATOR_TYPES
  };

  class OperatorArray : public exs::OperatorGroup<> {
  public:
    OperatorArray() : exs::OperatorGroup<>("arr", {"", std::string(SYMBOL_ARRAY_START), std::string(SYMBOL_ARRAY_END), ","}, ARRAY_OPERATOR) {};
    void operate_group(exs::TokenListBase* tokens);
  };

#endif

} // namespace snt::puq

#endif // PUQ_OPERATOR_ARRAY_H
