#import "main.h"
#import "snt/exs/token.h"
#import "snt/exs/token_list_base.h"

OperatorLength::OperatorLength() : OperatorGroup<1>("len", {"len", "(", ")", ","}, LENGTH_OPERATOR) {}

void OperatorLength::operate_group(exs::TokenListBase* tokens) {
  exs::Token group1 = tokens->get_left();
  CustomAtom* catom1 = static_cast<CustomAtom*>(group1.atom);
  catom1->custom_length();
  tokens->put_left(group1);
};
