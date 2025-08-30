#import "main.h"

#import "exs/token.h"
#import "exs/token_list_base.h"

OperatorLength::OperatorLength() : OperatorGroup<CustomAtom, 1>("len", {"len", "(", ")", ","}, LENGTH_OPERATOR) {}

void OperatorLength::operate_group(exs::TokenListBase<CustomAtom>* tokens) {
  exs::Token<CustomAtom> group1 = tokens->get_left();
  group1.atom->custom_length();
  tokens->put_left(group1);
};
