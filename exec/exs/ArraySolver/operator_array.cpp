#import "main.h"
#import "snt/exs/token.h"
#import "snt/exs/token_list_base.h"

#import <algorithm>

void OperatorArray::operate_group(exs::TokenListBase* tokens) {
  exs::Token group1 = tokens->get_left();
  CustomAtom* catom1 = static_cast<CustomAtom*>(group1.atom);
  for (size_t i = 1; i < num_groups; i++) {
    exs::Token group2 = tokens->get_left();
    CustomAtom* catom2 = static_cast<CustomAtom*>(group2.atom);
    catom1->value.push_back(catom2->value[0]);    
  }
  std::reverse(catom1->value.begin(), catom1->value.end());
  tokens->put_left(group1);
};
