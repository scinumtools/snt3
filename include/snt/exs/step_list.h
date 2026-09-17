#ifndef EXS_STEP_LIST_H
#define EXS_STEP_LIST_H

#include <memory>
#include <snt/exs/settings.h>
#include <vector>

namespace snt::exs {

    using StepListType = std::vector<std::pair<OperationType, std::vector<int>>>;

    /** Ordered execution steps generated from an EXS expression. */
    class StepList {
      public:
        StepListType steps;
        /** Append one execution step.
         * @param t Operation category to execute.
         * @param s Operator identifiers executed for this step.
         */
        void append(OperationType t, const std::vector<int>& s);
    };

} // namespace snt::exs

#endif // EXS_STEP_LIST_H
