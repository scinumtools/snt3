#ifndef EXS_OPERATOR_LIST_H
#define EXS_OPERATOR_LIST_H

#include <memory>
#include <snt/exs/operator_base.h>
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
