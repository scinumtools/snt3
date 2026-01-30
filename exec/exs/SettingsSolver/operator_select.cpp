#import "main.h"

OperatorSelect::OperatorSelect() : OperatorGroup<1>("sel", {"", "{", "}", ","}, SELECT_OPERATOR) {}

void OperatorSelect::operate_group(exs::TokenListBase* tokens, exs::BaseSettings* set) {
  Settings* cset = static_cast<Settings*>(set);
  exs::Token group1 = tokens->get_left();
  // substitude value from an option list
  CustomAtom* catom1 = static_cast<CustomAtom*>(group1.atom);
  catom1->value = cset->options.at(catom1->value);
  tokens->put_left(group1);
};
