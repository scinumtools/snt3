#ifndef EXS_STEP_LIST_H
#define EXS_STEP_LIST_H

#include <memory>
#include <snt/exs/settings.h>
#include <vector>

namespace snt::exs {

  typedef std::vector<std::pair<OperationType, std::vector<int>>> StepListType;

  class StepList {
  public:
    StepListType steps;
    void append(OperationType t, const std::vector<int>& s);
  };

} // namespace snt::exs

#endif // EXS_STEP_LIST_H
