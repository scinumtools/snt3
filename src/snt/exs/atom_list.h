#ifndef EXS_ATOM_LIST_H
#define EXS_ATOM_LIST_H

#include "settings.h"

#include <memory>

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class AtomList {
  public:
    std::vector<std::unique_ptr<AtomGrand>> atoms;
    //AtomGrand* append(std::string s, S* set = nullptr) {
    //  if constexpr (!std::is_same_v<S, EmptySettings>) {
    //    atoms.push_back(std::make_unique<A>(A::from_string(s, set)));
    //  } else {
    //    atoms.push_back(std::make_unique<A>(A::from_string(s)));
    //  }
    //  return atoms.back().get();
    //};
    AtomGrand* append(std::unique_ptr<AtomGrand> a) {
      atoms.push_back(std::move(a));
      return atoms.back().get();
    };
  };

} // namespace snt::exs

#endif // EXS_ATOM_LIST_H
