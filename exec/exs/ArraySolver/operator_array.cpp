#import "main.h"

#import <algorithm>

void OperatorArray::operate_group(exs::TokenListBase<CustomAtom>* tokens) {
  exs::Token<CustomAtom> group1 = tokens->get_left();
  for (size_t i = 1; i < num_groups; i++) {
    exs::Token<CustomAtom> group2 = tokens->get_left();
    group1.atom->value.push_back(group2.atom->value[0]);
  }
  std::reverse(group1.atom->value.begin(), group1.atom->value.end());
  tokens->put_left(group1);
};
