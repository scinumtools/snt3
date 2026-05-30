#include <snt/val/values_array.h>

namespace snt::val {

    BaseValue::PointerType ArrayValue<std::string>::cast_as(core::DataType dt) const {
        switch (dt) {
        case core::DataType::Boolean: {
            std::vector<uint8_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++) {
                if (this->value[i] == core::KEYWORD_TRUE)
                    arr[i] = true;
                else if (this->value[i] == core::KEYWORD_FALSE)
                    arr[i] = false;
                else
                    throw std::runtime_error("Could not cast value as boolean: " + this->value[i]);
            }
            return std::make_unique<ArrayValue<uint8_t>>(arr, this->shape, dt);
        }
        case core::DataType::Character: {
            std::vector<int8_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                if (this->value[i].size() == 1)
                    arr[i] = this->value[i][0];
                else
                    throw std::runtime_error("Could not cast value as character: " + this->value[i]);
            return std::make_unique<ArrayValue<int8_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer16: {
            std::vector<int64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = (int64_t)std::stoi(this->value[i]);
            return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer32: {
            std::vector<int64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++) {
                long n = std::stol(this->value[i]);
                if (n < INT32_MIN || n > INT32_MAX)
                    throw std::out_of_range("Value out of int32_t range");
                arr[i] = static_cast<int64_t>(n);
            }
            return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer64: {
            std::vector<int64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++) {
                long long n = std::stoll(this->value[i]);
                if (n < INT64_MIN || n > INT64_MAX)
                    throw std::out_of_range("Value out of int64_t range");
                arr[i] = static_cast<int64_t>(n);
            }
            return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer16_U: {
            std::vector<uint64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = (uint64_t)std::stoi(this->value[i]);
            return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer32_U: {
            std::vector<uint64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++) {
                unsigned long n = std::stoul(this->value[i]);
                if (n > UINT32_MAX)
                    throw std::out_of_range("Value out of uint32_t range");
                arr[i] = static_cast<uint64_t>(n);
            }
            return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Integer64_U: {
            std::vector<uint64_t> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++) {
                unsigned long long n = std::stoull(this->value[i]);
                if (n > UINT64_MAX)
                    throw std::out_of_range("Value out of uint64_t range");
                arr[i] = static_cast<uint64_t>(n);
            }
            return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
        }
        case core::DataType::Float32: {
            std::vector<double> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = std::stof(this->value[i]);
            return std::make_unique<ArrayValue<double>>(arr, this->shape, dt);
        }
        case core::DataType::Float64: {
            std::vector<double> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = std::stod(this->value[i]);
            return std::make_unique<ArrayValue<double>>(arr, this->shape, dt);
        }
        case core::DataType::Float128: {
            std::vector<long double> arr(this->value.size());
            for (size_t i = 0; i < this->value.size(); i++)
                arr[i] = std::stold(this->value[i]);
            return std::make_unique<ArrayValue<long double>>(arr, this->shape, dt);
        }
        case core::DataType::String: {
            return std::make_unique<ArrayValue<std::string>>(this->value, this->shape, dt);
        }
        default:
            throw std::runtime_error("Not implemented");
            return nullptr;
        }
    };

    BaseValue::PointerType ArrayValue<std::string>::logical_and(const BaseValue* other) const {
        return operate_binary<uint8_t>(
            other,
            [](const std::string& a, const std::string& b) { return !a.empty() && !b.empty(); },
            core::DataType::Boolean
        );
    }

    BaseValue::PointerType ArrayValue<std::string>::logical_or(const BaseValue* other) const {
        return operate_binary<uint8_t>(
            other,
            [](const std::string& a, const std::string& b) { return !a.empty() || !b.empty(); },
            core::DataType::Boolean
        );
    };

    BaseValue::PointerType ArrayValue<std::string>::logical_not() const {
        return operate_unary<uint8_t>([](const std::string& a) { return a.empty(); }, core::DataType::Boolean);
    };

} // namespace snt::val
