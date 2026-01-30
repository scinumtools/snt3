#ifndef EXS_ATOM_LIST_H
#define EXS_ATOM_LIST_H

#include "settings.h"

#include <memory>

namespace snt::exs {

  class AtomList {
  public:
    std::vector<std::unique_ptr<AtomGrand>> atoms;
    AtomGrand* append(std::unique_ptr<AtomGrand> a) {
      atoms.push_back(std::move(a));
      return atoms.back().get();
    };
  };

} // namespace snt::exs

#endif // EXS_ATOM_LIST_H
