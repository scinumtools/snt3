#include <snt/exs/operator_list.h>

namespace snt::exs {

  OperatorBase* OperatorList::select(int otype) {
    auto it = operators.find(otype);
    if (it != operators.end()) {
      return &(*(it->second));
    } else {
      throw std::logic_error("Selecting non existing operator");
    }
  };
  
  void OperatorList::append(int t, std::shared_ptr<OperatorBase> o) {
    operators[t] = o;
    order.push_back(t);
  };

}
