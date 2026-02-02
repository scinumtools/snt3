#include "step_list.h"

namespace snt::exs {

  void StepList::append(OperationType t, std::vector<int> s) {
    steps.push_back({t, s});
  };

}
