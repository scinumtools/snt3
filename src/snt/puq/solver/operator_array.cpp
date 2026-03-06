#include "operator_array.h"

namespace snt::puq {

#if defined(MAGNITUDE_VALUES)

  // TODO: this needs probably templates because of the ArrayValue
  void OperatorArray::operate_group(exs::TokenListBase* tokens) {
    exs::Token group;
    std::vector<double> nv(num_groups);
    std::vector<double> ne(num_groups);
    bool hasUncertainties = false;
    for (size_t i = 0; i < num_groups; i++) {
      group = tokens->get_left();
      UnitAtom* atom = static_cast<UnitAtom*>(group.atom);
      val::ArrayValue<double> value(atom->value.magnitude.estimate.get());
      nv[i] = value.get_value(0);
      if (atom->value.magnitude.uncertainty) {
        hasUncertainties = true;
        val::ArrayValue<double> uncertainty(atom->value.magnitude.uncertainty.get());
        ne[i] = uncertainty.get_value(0);
      }
    }
    std::reverse(nv.begin(), nv.end());
    if (group.atom) {
      UnitAtom* atom = static_cast<UnitAtom*>(group.atom);
      atom->value.magnitude.estimate = std::make_unique<val::ArrayValue<double>>(nv);
      if (hasUncertainties) {
        std::reverse(ne.begin(), ne.end());
        atom->value.magnitude.uncertainty = std::make_unique<val::ArrayValue<double>>(ne);
      } else {
        atom->value.magnitude.uncertainty = nullptr;
      }
    } else {
      throw std::runtime_error("Detected empty atom in a group");
    }
    tokens->put_left(group);
  };

#endif

} // namespace snt::puq
