#ifndef EXS_OPERATOR_TRIGONOMETRY_H
#define EXS_OPERATOR_TRIGONOMETRY_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

  // sinus

  class OperatorSinus : public OperatorGroup<1> {
  public:
    OperatorSinus(const OperatorGroupSybols& s = {"sin", "(", ")", ","});

    void operate_group(TokenListBase* tokens) override;
  };

  // cosinus

  class OperatorCosinus : public OperatorGroup<1> {
  public:
    OperatorCosinus(const OperatorGroupSybols& s = {"cos", "(", ")", ","});
    void operate_group(TokenListBase* tokens) override;
  };

  // tangens

  class OperatorTangens : public OperatorGroup<1> {
  public:
    OperatorTangens(const OperatorGroupSybols& s = {"tan", "(", ")", ","});

    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_TRIGONOMETRY_H
