#ifndef EXS_ATOM_LIST_H
#define EXS_ATOM_LIST_H

#include "settings.h"
#include "atom.h"

#include <memory>
#include <vector>

namespace snt::exs {

  class AtomList {
  public:
    std::vector<std::unique_ptr<AtomGrand>> atoms;
    AtomGrand* append(std::unique_ptr<AtomGrand> a);
  };

} // namespace snt::exs

#endif // EXS_ATOM_LIST_H
