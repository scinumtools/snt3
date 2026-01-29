#import "main.h"

OperatorSelect::OperatorSelect() : OperatorGroup<1, Settings>("sel", {"", "{", "}", ","}, SELECT_OPERATOR) {}

void OperatorSelect::operate_group(exs::TokenListBase* tokens, Settings* settings) {
  exs::Token group1 = tokens->get_left();
  // substitude value from an option list
  CustomAtom* catom1 = static_cast<CustomAtom*>(group1.atom);
  catom1->value = settings->options.at(catom1->value);
  tokens->put_left(group1);
};
