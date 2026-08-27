#include <snt/exs/exceptions.h>
#include <snt/exs/operator_list.h>

namespace snt::exs {

    OperatorBase* OperatorList::select(int otype) {
        auto it = operators.find(otype);
        if (it != operators.end()) {
            return &(*(it->second));
        } else {
            throw exs::ParserException(
                "Operator not found",
                "The requested operator does not exist in the operator list.",
                "Use an operator type that is registered in the parser.",
                __FILE__,
                __LINE__
            );
        }
    };

    void OperatorList::append(int t, std::shared_ptr<OperatorBase> o) {
        operators[t] = std::move(o);
        order.push_back(t);
    };

} // namespace snt::exs
