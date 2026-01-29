#ifndef EXS_OPERATOR_SQUARE_ROOT_H
#define EXS_OPERATOR_SQUARE_ROOT_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorSquareRoot : public OperatorGroup<1, S> {
  public:
    OperatorSquareRoot(const OperatorGroupSybols& s = {"sqrt", "(", ")", ","}) : OperatorGroup<1, S>("sqrt", s, SQUARE_ROOT_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group1 = tokens->get_left();
      group1.atom->math_square_root();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_SQUARE_ROOT_H
