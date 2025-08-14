#ifndef EXS_STEP_LIST_H
#define EXS_STEP_LIST_H

#include <memory>
#include <vector>

namespace exs {

  typedef std::vector<std::pair<OperationType, std::vector<int>>> StepListType;

  class StepList {
  public:
    StepListType steps;
    void append(OperationType t, std::vector<int> s) {
      steps.push_back({t, s});
    };
  };

} // namespace exs

#endif // EXS_STEP_LIST_H
