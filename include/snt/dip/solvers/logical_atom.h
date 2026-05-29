#ifndef DIP_LOGICAL_ATOM_H
#define DIP_LOGICAL_ATOM_H

#include <snt/dip/environment.h>

namespace snt::dip {

    struct LogicalSettings : exs::BaseSettings {
        Environment* env;
    };

    class LogicalAtom : public exs::AtomBase<LogicalAtom, ValueNodeData> {
      public:
        // Constructor from unique_ptr
        LogicalAtom(ValueNodeData b) : AtomBase({std::move(b.value), std::move(b.units)}) {};
        // Deep copy constructor
        LogicalAtom(const LogicalAtom& a)
            : AtomBase({a.value.value ? a.value.value->clone() : nullptr, a.value.units}) {};
        LogicalAtom& operator=(const LogicalAtom& a);
        // Move constructor
        LogicalAtom(LogicalAtom&& a) noexcept = default;
        LogicalAtom& operator=(LogicalAtom&& a) noexcept = default;

        static ValueNodeData from_string(const std::string& s, exs::BaseSettings* settings);
        std::string to_string() override;
        void comparison_equal(LogicalAtom* other) override;
        void comparison_not_equal(LogicalAtom* other) override;
        void comparison_less_equal(LogicalAtom* other) override;
        void comparison_greater_equal(LogicalAtom* other) override;
        void comparison_less(LogicalAtom* other) override;
        void comparison_greater(LogicalAtom* other) override;
        void logical_not() override;
        void logical_and(LogicalAtom* other) override;
        void logical_or(LogicalAtom* other) override;
        void custom_defined();
        void custom_not_defined();
    };

} // namespace snt::dip

#endif // DIP_LOGICAL_ATOM_H
