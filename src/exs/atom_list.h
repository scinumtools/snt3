#ifndef EXS_ATOM_LIST_H
#define EXS_ATOM_LIST_H

#include "settings.h"

#include <memory>

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class AtomList {
  public:
    std::vector<std::shared_ptr<A>> atoms;
    A* append(std::string s, S* set = nullptr) {
      if constexpr (!std::is_same_v<S, EmptySettings>) {
        atoms.push_back(std::make_shared<A>(A::from_string(s, set)));
      } else {
        atoms.push_back(std::make_shared<A>(A::from_string(s)));
      }
      return &(*atoms.back());
    };
    A* append(A a) {
      atoms.push_back(std::make_shared<A>(a));
      return &(*atoms.back());
    };
  };

} // namespace snt::exs

#endif // EXS_ATOM_LIST_H
