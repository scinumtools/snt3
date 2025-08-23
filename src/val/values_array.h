#ifndef VAL_VALUES_ARRAY_H
#define VAL_VALUES_ARRAY_H

#include "values.h"

#include <typeinfo>

namespace val {

  // here we need a forward declaration
  template <typename T>
  class BaseArrayValue;
  template <typename T>
  class ArrayValue;

  template <typename T>
  class BaseArrayValue : public BaseValue {
  protected:
    std::vector<T> value;

  public:
    BaseArrayValue(const T& val) : value({val}), BaseValue(this->deduce_dtype(), {1}) {};
    BaseArrayValue(const std::vector<T>& arr, const Array::ShapeType& sh) : value(arr), BaseValue(this->deduce_dtype(), sh) {};
    BaseArrayValue(const BaseValue* other) : BaseValue(this->deduce_dtype(), other->get_shape()) {
      const BaseArrayValue<T>* otherT;
      BaseValue::PointerType casted_other;
      if (this->dtype == other->get_dtype()) {
        otherT = dynamic_cast<const ArrayValue<T>*>(other);
      } else {
        casted_other = other->cast_as(this->dtype);
        otherT = dynamic_cast<const ArrayValue<T>*>(casted_other.get());
      }
      if (otherT) {
        this->value = otherT->value;
      } else
        throw std::runtime_error("ArrayValue could not be initialized from the given BaseValue.");
    };
    static BaseValue::PointerType pointer_from_vector(const std::vector<T>& val) {
      return std::make_unique<val::ArrayValue<T>>(std::vector<T>(val));
    };
    void print() override { std::cout << to_string() << std::endl; };
    std::vector<T> get_values() const { return value; };
    T get_value(const size_t index) const { return value.at(index); };
    size_t get_size() const override { return value.size(); };

  protected:
    static constexpr DataType deduce_dtype() {
      if constexpr (std::is_same_v<T, bool>) {
        return DataType::Boolean;
      } else if constexpr (std::is_same_v<T, char>) {
        return DataType::Character;
      } else if constexpr (std::is_same_v<T, int16_t>) {
        return DataType::Integer16;
      } else if constexpr (std::is_same_v<T, int32_t>) {
        return DataType::Integer32;
      } else if constexpr (std::is_same_v<T, int64_t>) {
        return DataType::Integer64;
      } else if constexpr (std::is_same_v<T, uint16_t>) {
        return DataType::Integer16_U;
      } else if constexpr (std::is_same_v<T, uint32_t>) {
        return DataType::Integer32_U;
      } else if constexpr (std::is_same_v<T, uint64_t>) {
        return DataType::Integer64_U;
      } else if constexpr (std::is_same_v<T, float>) {
        return DataType::Float32;
      } else if constexpr (std::is_same_v<T, double>) {
        return DataType::Float64;
      } else if constexpr (std::is_same_v<T, long double>) {
        return DataType::Float128;
      } else if constexpr (std::is_same_v<T, std::string>) {
        return DataType::String;
      } else {
        static_assert(sizeof(T) == 0, "Unsupported type T");
      }
    }
    virtual void value_to_string(std::ostringstream& oss, size_t& offset, int precision = 0) const = 0;
    std::string to_string_dim(size_t& offset, const int& precision = 0, int dim = 0) const {
      std::ostringstream oss;
      if (value.size() > 1)
        oss << "[";
      for (int i = 0; i < shape[dim]; i++) {
        if (i > 0)
          oss << ", ";
        if (dim + 1 < shape.size()) {
          oss << to_string_dim(offset, precision, dim + 1);
        } else {
          value_to_string(oss, offset, precision);
          offset++;
        }
      }
      if (value.size() > 1)
        oss << "]";
      return oss.str();
    }

  public:
    std::string to_string(const int precision = 0) const override {
      size_t offset = 0;
      if (precision == 0) {
        return to_string_dim(offset);
      } else {
        return to_string_dim(offset, precision);
      }
    };
    /*
     * Operations
     */
  protected:
    template <typename R, typename Func>
    std::unique_ptr<ArrayValue<R>> operate_unary(Func f) const {
      // apply operation
      std::vector<R> arr(value.size());
      for (int i = 0; i < value.size(); i++)
        arr[i] = f(value[i]);
      return std::make_unique<ArrayValue<R>>(arr, this->shape);
    };
    template <typename R, typename Func>
    void operate_unary_equal(Func f) {
      // apply operation
      for (int i = 0; i < value.size(); i++)
        value[i] = f(value[i]);
    };
    template <typename R, typename Func>
    std::unique_ptr<ArrayValue<R>> operate_binary(const BaseValue* other, Func f) const {
      std::vector<R> arr;
      if (this->get_size() == 1 && other->get_size() == 1) { // both are scalars
        const ArrayValue<T> otherT(other);
        return std::make_unique<ArrayValue<R>>(f(this->get_value(0), otherT.get_value(0)));
      } else if (this->get_size() == 1) { // left is a scalar
        const ArrayValue<T> otherT(other);
        arr.reserve(other->get_size());
        for (int i = 0; i < other->get_size(); i++)
          arr.push_back(f(this->get_value(0), otherT.get_value(i)));
        return std::make_unique<ArrayValue<R>>(arr, otherT.get_shape());
      } else if (other->get_size() == 1) { // right is a scalar
        const ArrayValue<T> otherT(other);
        arr.reserve(this->get_size());
        for (int i = 0; i < this->get_size(); i++)
          arr.push_back(f(this->get_value(i), otherT.get_value(0)));
        return std::make_unique<ArrayValue<R>>(arr, this->get_shape());
      } else { // both are arrays
        // test if shape match
        if (shape.size() != other->get_shape().size()) // Compare shape size
          throw std::runtime_error("Arrays have incompatible shapes");
        for (int i = 0; i < shape.size(); i++) // Compare shape values
          if (shape[i] != other->get_shape()[i])
            throw std::runtime_error("Arrays have incompatible shapes");
        // apply operation
        const ArrayValue<T> otherT(other);
        arr.reserve(this->get_size());
        for (int i = 0; i < this->get_size(); i++)
          arr.push_back(f(this->get_value(i), otherT.get_value(i)));
        return std::make_unique<ArrayValue<R>>(arr, this->get_shape());
      }
    };
    template <typename R, typename Func>
    void operate_binary_equal(const BaseValue* other, Func f) {
      if (this->get_size() == 1 && other->get_size() == 1) { // both are scalars
        const ArrayValue<T> otherT(other);
        value[0] = f(this->get_value(0), otherT.get_value(0));
      } else if (this->get_size() == 1) { // left is a scalar
        const ArrayValue<T> otherT(other);
        T val = this->get_value(0);
        value.clear();
        value.reserve(otherT.get_size());
        shape = otherT.get_shape();
        for (int i = 0; i < otherT.get_size(); i++)
          value.push_back(f(val, otherT.get_value(i)));
      } else if (other->get_size() == 1) { // right is a scalar
        const ArrayValue<T> otherT(other);
        for (int i = 0; i < this->get_size(); i++)
          value[i] = f(this->get_value(i), otherT.get_value(0));
      } else { // both are arrays
        // test if shape match
        if (shape.size() != other->get_shape().size()) // Compare shape size
          throw std::runtime_error("Arrays have incompatible shapes");
        for (int i = 0; i < shape.size(); i++) // Compare shape values
          if (shape[i] != other->get_shape()[i])
            throw std::runtime_error("Arrays have incompatible shapes");
        // apply operation
        const ArrayValue<T> otherT(other);
        for (int i = 0; i < this->get_size(); i++)
          value[i] = f(this->get_value(i), otherT.get_value(i));
      }
    };
    template <typename U, typename R, typename Func>
    std::unique_ptr<ArrayValue<R>> operate_ternary(const BaseValue* other1, const BaseValue* other2, Func f) const {
      // test if shape match
      if (shape.size() != other1->get_shape().size() || shape.size() != other2->get_shape().size()) // Compare shape size
        throw std::runtime_error("Arrays have incompatible shapes");
      for (int i = 0; i < shape.size(); i++) { // Compare shape values
        if (shape[i] != other1->get_shape()[i])
          throw std::runtime_error("First other has incompatible shape");
        if (shape[i] != other2->get_shape()[i])
          throw std::runtime_error("Second other has incompatible shape");
      }
      // apply operation
      const ArrayValue<U> other1T(other1);
      const ArrayValue<R> other2T(other2);
      std::vector<R> arr(value.size());
      for (int i = 0; i < value.size(); i++)
        arr[i] = f(other1T.get_value(i), value[i], other2T.value[i]);
      return std::make_unique<ArrayValue<R>>(arr, this->shape);
    };

  public:
    BaseValue::PointerType compare_equal(const BaseValue* other) const override {
      return operate_binary<bool>(other, [](T a, T b) { return a == b; });
    };
    BaseValue::PointerType compare_not_equal(const BaseValue* other) const override {
      return operate_binary<bool>(other, [](T a, T b) { return a != b; });
    };
    BaseValue::PointerType compare_less(const BaseValue* other) const override {
      return operate_binary<bool>(other, [](T a, T b) { return a < b; });
    };
    BaseValue::PointerType compare_greater(const BaseValue* other) const override {
      return operate_binary<bool>(other, [](T a, T b) { return a > b; });
    };
    BaseValue::PointerType compare_less_equal(const BaseValue* other) const override {
      return operate_binary<bool>(other, [](T a, T b) { return a <= b; });
    };
    BaseValue::PointerType compare_greater_equal(const BaseValue* other) const override {
      return operate_binary<bool>(other, [](T a, T b) { return a >= b; });
    };
    /*
     * Array slicing
     */
    BaseValue::PointerType slice_value(const Array::RangeType& slice) {
      if (slice.size() != this->shape.size())
        throw std::runtime_error("Array slice size does not correspond with array shape: " + std::to_string(slice.size()) + "!=" + std::to_string(this->shape.size()));
      // calculate new shape and size
      Array::ShapeType new_shape;
      size_t new_size = 0;
      for (size_t i = 0; i < this->shape.size(); i++) {
        int dmin = slice[i].dmin;
        int dmax = (slice[i].dmax == Array::max_range) ? this->shape[i] - 1 : slice[i].dmax;
        int new_dim = dmax + 1 - dmin;
        if (new_dim > 1) {
          new_shape.push_back(new_dim);
          new_size += new_dim;
        }
      }
      // allocate variables
      std::vector<T> new_value;
      new_value.reserve(new_size);
      Array::ShapeType coord(this->shape.size(), 0);
      // list through all values and copy sliced
      for (size_t i = 0; i < this->value.size(); i++) {
        // copy only values within the given ranges
        bool push = true;
        for (size_t dim = 0; dim < this->shape.size(); dim++) {
          int dmin = slice[dim].dmin;
          int dmax = (slice[dim].dmax == Array::max_range) ? this->shape[dim] - 1 : slice[dim].dmax;
          if (coord[dim] < dmin or dmax < coord[dim])
            push = false;
        }
        if (push)
          new_value.push_back(this->value[i]);
        // increase the coodrinates
        for (size_t dim = this->shape.size(); dim-- > 0;) {
          if (++coord[dim] < this->shape[dim])
            break;
          coord[dim] = 0; // carry over
        }
      }
      if (new_size > 1)
        return std::make_unique<ArrayValue<T>>(new_value, new_shape);
      else
        return std::make_unique<ArrayValue<T>>(new_value[0]);
    };
    BaseValue::PointerType math_sin() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_cos() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_tan() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_log() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_log10() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_sqrt() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_cbrt() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_exp() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_floor() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_ceil() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_abs() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_neg() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_add(const BaseValue* other) const override { throw std::runtime_error("Function is not implemented for this type."); };
    void math_add_equal(const BaseValue* other) override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_sub(const BaseValue* other) const override { throw std::runtime_error("Function is not implemented for this type."); };
    void math_sub_equal(const BaseValue* other) override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_inv() const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_mul(const BaseValue* other) const override { throw std::runtime_error("Function is not implemented for this type."); };
    void math_mul_equal(const BaseValue* other) override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_div(const BaseValue* other) const override { throw std::runtime_error("Function is not implemented for this type."); };
    void math_div_equal(const BaseValue* other) override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_pow(const BaseValue* other) const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_pow(const double exp) const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_max(const BaseValue* other) const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType math_min(const BaseValue* other) const override { throw std::runtime_error("Function is not implemented for this type."); };
    BaseValue::PointerType where(const BaseValue* condition, const BaseValue* other) const override {
      return operate_ternary<bool, T>(condition, other, [](bool c, T a, T b) { return c ? a : b; });
    };
  };

} // namespace val

#endif // VAL_VALUES_ARRAY_H
