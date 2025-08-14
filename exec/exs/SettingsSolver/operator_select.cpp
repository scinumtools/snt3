#import "main.h"

OperatorSelect::OperatorSelect() : OperatorGroup<CustomAtom, 1, Settings>("sel", "{", SELECT_OPERATOR, "{", "}") {}

void OperatorSelect::operate_group(exs::TokenListBase<CustomAtom>* tokens, Settings* settings) {
  exs::Token<CustomAtom> group1 = tokens->get_left();
  // substitude value from an option list
  group1.atom->value = settings->options.at(group1.atom->value);
  tokens->put_left(group1);
};
