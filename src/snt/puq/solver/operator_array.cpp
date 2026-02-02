#include "operator_array.h"

namespace snt::puq {

#if defined(MAGNITUDE_VALUES)

  // TODO: this needs probably templates because of the ArrayValue
  void OperatorArray::operate_group(exs::TokenListBase* tokens) {
    exs::Token group;
#ifdef MAGNITUDE_ERRORS
    std::vector<double> nv(num_groups);
    std::vector<double> ne(num_groups);
    bool hasErrors = false;
    for (size_t i = 0; i < num_groups; i++) {
      group = tokens->get_left();
      UnitAtom* atom = static_cast<UnitAtom*>(group.atom);
      val::ArrayValue<double> value(atom->value.magnitude.value.get());
      nv[i] = value.get_value(0);
      if (atom->value.magnitude.error) {
        hasErrors = true;
        val::ArrayValue<double> error(atom->value.magnitude.error.get());
        ne[i] = error.get_value(0);
      }
    }
    std::reverse(nv.begin(), nv.end());
    if (group.atom) {
      UnitAtom* atom = static_cast<UnitAtom*>(group.atom);
      atom->value.magnitude.value = std::make_unique<val::ArrayValue<double>>(nv);
      if (hasErrors) {
        std::reverse(ne.begin(), ne.end());
        atom->value.magnitude.error = std::make_unique<val::ArrayValue<double>>(ne);
      } else {
        atom->value.magnitude.error = nullptr;
      }
    } else {
      throw std::runtime_error("Detected empty atom in a group");
    }
#else
    std::vector<double> nv(num_groups);
    for (size_t i = 0; i < num_groups; i++) {
      group = tokens->get_left();
      UnitAtom* atom = static_cast<UnitAtom*>(group.atom);
      val::ArrayValue<double> value(atom->value.magnitude.get());
      nv[i] = value.get_value(0);
    }
    std::reverse(nv.begin(), nv.end());
    UnitAtom* atom = static_cast<UnitAtom*>(group.atom);
    atom->value.magnitude = std::make_unique<val::ArrayValue<double>>(nv);
#endif
    tokens->put_left(group);
  };

#endif

} // namespace snt::puq
