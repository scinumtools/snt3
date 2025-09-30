#include "operator_array.h"

namespace snt::puq {

#if defined(MAGNITUDE_ARRAYS)

  void OperatorArray::operate_group(exs::TokenListBase<UnitAtom>* tokens) {
    exs::Token<UnitAtom> group;
#ifdef MAGNITUDE_ERRORS
    ArrayValue nv(num_groups);
    ArrayValue ne(num_groups);
    for (size_t i = 0; i < num_groups; i++) {
      group = tokens->get_left();
      nv[i] = group.atom->value.magnitude.value[0];
      ne[i] = group.atom->value.magnitude.error[0];
    }
    std::reverse(nv.begin(), nv.end());
    std::reverse(ne.begin(), ne.end());
    group.atom->value.magnitude.value = Array(nv, {nv.size()});
    group.atom->value.magnitude.error = Array(ne, {nv.size()});
#else
    ArrayValue nv(num_groups);
    for (size_t i = 0; i < num_groups; i++) {
      group = tokens->get_left();
      nv[i] = group.atom->value.magnitude[0];
    }
    std::reverse(nv.begin(), nv.end());
    group.atom->value.magnitude = Array(nv);
#endif
    tokens->put_left(group);
  };

#elif defined(MAGNITUDE_VALUES)

  // TODO: this needs probably templates because of the ArrayValue
  void OperatorArray::operate_group(exs::TokenListBase<UnitAtom>* tokens) {
    exs::Token<UnitAtom> group;
#ifdef MAGNITUDE_ERRORS
    std::vector<double> nv(num_groups);
    std::vector<double> ne(num_groups);
    bool hasErrors = false;
    for (size_t i = 0; i < num_groups; i++) {
      group = tokens->get_left();
      val::ArrayValue<double> value(group.atom->value.magnitude.value.get());
      nv[i] = value.get_value(0);
      if (group.atom->value.magnitude.error) {
        hasErrors = true;
        val::ArrayValue<double> error(group.atom->value.magnitude.error.get());
        ne[i] = error.get_value(0);
      }
    }
    std::reverse(nv.begin(), nv.end());
    if (group.atom) {
      group.atom->value.magnitude.value = std::make_unique<val::ArrayValue<double>>(nv);
      if (hasErrors) {
        std::reverse(ne.begin(), ne.end());
        group.atom->value.magnitude.error = std::make_unique<val::ArrayValue<double>>(ne);
      } else {
        group.atom->value.magnitude.error = nullptr;
      }
    } else {
      throw std::runtime_error("Detected empty atom in a group");
    }
#else
    std::vector<double> nv(num_groups);
    for (size_t i = 0; i < num_groups; i++) {
      group = tokens->get_left();
      val::ArrayValue<double> value(group.atom->value.magnitude.get());
      nv[i] = value.get_value(0);
    }
    std::reverse(nv.begin(), nv.end());
    group.atom->value.magnitude = std::make_unique<val::ArrayValue<double>>(nv);
#endif
    tokens->put_left(group);
  };

#endif

} // namespace snt::puq
