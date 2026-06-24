#include "snt/exs/token.h"
#include "snt/exs/token_list_base.h"

#include <snt/dip/solvers/logical_atom.h>
#include <snt/dip/solvers/logical_operators.h>

namespace snt::dip {

    OperatorDefined::OperatorDefined()
        : exs::OperatorGroup<1>("def", {"def", "( ", " )", ","}, dip::DEFINED_OPERATOR) {}

    void OperatorDefined::operate_group(exs::TokenListBase* tokens) {
        exs::Token group1 = tokens->get_left();
        LogicalAtom* catom1 = static_cast<LogicalAtom*>(group1.atom);
        catom1->custom_defined();
        tokens->put_left(group1);
    };

    OperatorNotDefined::OperatorNotDefined()
        : exs::OperatorGroup<1>("ndef", {"ndef", "( ", " )", ","}, dip::NOT_DEFINED_OPERATOR) {}

    void OperatorNotDefined::operate_group(exs::TokenListBase* tokens) {
        exs::Token group1 = tokens->get_left();
        LogicalAtom* catom1 = static_cast<LogicalAtom*>(group1.atom);
        catom1->custom_not_defined();
        tokens->put_left(group1);
    };

} // namespace snt::dip
