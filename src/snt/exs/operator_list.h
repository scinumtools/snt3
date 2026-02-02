#ifndef EXS_OPERATOR_LIST_H
#define EXS_OPERATOR_LIST_H

#include "operator_base.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace snt::exs {

  class OperatorList {
    typedef std::unordered_map<int, std::shared_ptr<OperatorBase>> OperatorListType;

  public:
    OperatorListType operators;
    std::vector<int> order;
    OperatorBase* select(int otype);
    void append(int t, std::shared_ptr<OperatorBase> o);
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LIST_H
