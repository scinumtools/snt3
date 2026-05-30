#include <snt/val/values_array.h>

namespace snt::val {

    BaseValue::PointerType ArrayValue<bool>::cast_as(core::DataType dt) const {
        switch (dt) {
        case core::DataType::Boolean: {
            std::vector<uint8_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<uint8_t>(this->value[i]);
            return std::make_unique<ArrayValue<uint8_t>>(arr, this->shape, dt);
        }
        case core::DataType::Character: {
            std::vector<int8_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<int8_t>(this->value[i]);
            return std::make_unique<ArrayValue<int8_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer16: {
            std::vector<int64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<int64_t>(this->value[i]);
            return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer32: {
            std::vector<int64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<int64_t>(this->value[i]);
            return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer64: {
            std::vector<int64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<int64_t>(this->value[i]);
            return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer16_U: {
            std::vector<uint64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<uint64_t>(this->value[i]);
            return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer32_U: {
            std::vector<uint64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<uint64_t>(this->value[i]);
            return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer64_U: {
            std::vector<uint64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<uint64_t>(this->value[i]);
            return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Float32: {
            std::vector<double> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<double>(this->value[i]);
            return std::make_unique<ArrayValue<double>>(arr, this->shape, dt);
        }
        case core::DataType::Float64: {
            std::vector<double> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<double>(this->value[i]);
            return std::make_unique<ArrayValue<double>>(arr, this->shape, dt);
        }
        case core::DataType::Float128: {
            std::vector<long double> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = static_cast<long double>(this->value[i]);
            return std::make_unique<ArrayValue<long double>>(arr, this->shape, dt);
        }
        case core::DataType::String: {
            std::vector<std::string> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++) {
                if (this->value[i])
                    arr[i] = std::string(core::KEYWORD_TRUE);
                else
                    arr[i] = std::string(core::KEYWORD_FALSE);
            }
            return std::make_unique<ArrayValue<std::string>>(arr, this->shape, dt);
        }
        default:
            throw std::runtime_error("Not implemented");
            return nullptr;
        }
    };

    BaseValue::PointerType ArrayValue<bool>::logical_and(const BaseValue* other) const {
        return operate_binary<bool>(other, [](bool a, bool b) { return a && b; });
    };

    BaseValue::PointerType ArrayValue<bool>::logical_or(const BaseValue* other) const {
        return operate_binary<bool>(other, [](bool a, bool b) { return a || b; });
    };

    BaseValue::PointerType ArrayValue<bool>::logical_not() const {
        return operate_unary<bool>([](bool a) { return !a; });
    };

} // namespace snt::val
