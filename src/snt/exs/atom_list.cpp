#include "atom_list.h"

namespace snt::exs {

  AtomGrand* AtomList::append(std::unique_ptr<AtomGrand> a) {
    atoms.push_back(std::move(a));
    return atoms.back().get();
  };

}
