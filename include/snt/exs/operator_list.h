#ifndef EXS_OPERATOR_LIST_H
#define EXS_OPERATOR_LIST_H

#include <memory>
#include <snt/exs/operator_base.h>
#include <unordered_map>
#include <vector>

namespace snt::exs {

    /** Registry of operators used by an EXS solver. */
    class OperatorList {
        using OperatorListType = std::unordered_map<int, std::shared_ptr<OperatorBase>>;

      public:
        OperatorListType operators;
        std::vector<int> order;
        /** Select an operator implementation by type.
         * @param otype Operator category to select.
         */
        OperatorBase* select(int otype);
        /** Append an item to the collection.
         * @param t Operator type identifier used as the registry key.
         * @param o Operator implementation to register; ownership is shared.
         */
        void append(int t, std::shared_ptr<OperatorBase> o);
    };

} // namespace snt::exs

#endif // EXS_OPERATOR_LIST_H
