#ifndef VAL_VALUES_ARRAY_H
#define VAL_VALUES_ARRAY_H

#include <snt/val/value_base.h>
#include <typeinfo>

namespace snt::val {

    // here we need a forward declaration
    template <typename T> class BaseArrayValue;
    template <typename T> class ArrayValue;

    /**
     * @class BaseArrayValue
     * @brief Base class for all array values
     *
     * @tparam T Data type of a values
     */
    template <typename T> class BaseArrayValue : public BaseValue {
      protected:
        std::vector<T> value; ///< Internal data container that holds flattend array values

      public:
        /**
         * @brief Base array value constructor from a scalar value
         *
         * @param val Scalar value
         * @param dtype Datatype flag
         */
        BaseArrayValue(const T& val, const core::DataType dtype) : value({val}), BaseValue(dtype, {1}) {};

        /**
         * @brief Base array value constructor from a flattened vector of values and a shape
         *
         * @param arr Vector of values
         * @param sh Array shape
         * @param dtype Datatype flag
         */
        BaseArrayValue(const std::vector<T>& arr, const Array::ShapeType& sh, const core::DataType dtype)
            : value(arr), BaseValue(dtype, sh) {};

        /**
         * @brief Base array value constructor from a BaseValue pointer
         *
         * @param other Pointer to a BaseValue object
         * @param dtype Datatype flag
         */
        BaseArrayValue(const BaseValue* other, const core::DataType dtype) : BaseValue(dtype, other->get_shape()) {
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

        /**
         * @brief Create unique pointer from a vector of values
         *
         * @param val Reference to a vector of values
         * @return Unique pointer of a vector of values
         */
        static BaseValue::PointerType pointer_from_vector(const std::vector<T>& val) {
            return std::make_unique<val::ArrayValue<T>>(std::vector<T>(val));
        };

        /**
         * @brief Print object as a string
         */
        void print() override { std::cout << to_string() << std::endl; };

        /**
         * @brief Get copy of the array values
         *
         * @return Flattened array of values
         */
        std::vector<T> get_values() const { return value; };

        /**
         * @brief Get copy of a particular value
         *
         * @param index Index of a value
         * @return Single value
         */
        T get_value(const size_t index) const { return value.at(index); };

        /**
         * @brief Get pointer to the internal data container
         *
         * @return Pointer to the value data
         */
        T* get_data() { return value.data(); };

        /**
         * @brief Get size of an array
         *
         * @return Size of the flattened array value
         */
        size_t get_size() const override { return value.size(); };

        /**
         * @brief Print value into a string stream
         *
         * @param os String stream
         * @param val Value object
         */
        friend std::ostream& operator<<(std::ostream& os, const ArrayValue<T>& val) { return os << val.to_string(); };

        /**
         * @brief Return string representation of this array object
         *
         * @param format Object with an output string format
         * @return String representation
         */
        std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override {
            return core::array_to_string(value, shape, dtype, format);
        };

        /*
         * Operations
         */
      protected:
        /**
         * @brief Perform unary operation on a array value
         *
         * @param f operation Function on self and other array value
         * @param output_dtype DataType flage of the output (needed for booleans)
         * @tparam R Output data type
         * @tparam Func Type of the operation function (is deduced)
         */
        template <typename R, typename Func>
        std::unique_ptr<ArrayValue<R>> operate_unary(Func f, core::DataType output_dtype = core::DataType::None) const {
            if (output_dtype == core::DataType::None)
                output_dtype = dtype;
            // apply operation
            std::vector<R> arr(value.size());
            for (int i = 0; i < value.size(); i++)
                arr[i] = static_cast<R>(f(value[i]));
            return std::make_unique<ArrayValue<R>>(arr, this->shape, output_dtype);
        };

        /**
         * @brief Perform unary assignment operation on a array value
         *
         * @param f operation Function on self and other array value
         * @tparam R Output data type
         * @tparam Func Type of the operation function (is deduced)
         */
        template <typename R, typename Func> void operate_unary_equal(Func f) {
            // apply operation
            for (int i = 0; i < value.size(); i++)
                value[i] = f(value[i]);
        };

        /**
         * @brief Perform binary operation on two array values
         *
         * @param other The right hand array value
         * @param f operation Function on self and other array value
         * @param output_dtype DataType flage of the output (needed for booleans)
         * @tparam R Output data type
         * @tparam Func Type of the operation function (is deduced)
         */
        template <typename R, typename Func>
        std::unique_ptr<ArrayValue<R>> operate_binary(
            const BaseValue* other, Func f, core::DataType output_dtype = core::DataType::None
        ) const {
            if (output_dtype == core::DataType::None)
                output_dtype = dtype;
            std::vector<R> arr;
            if (this->get_size() == 1 && other->get_size() == 1) { // both are scalars
                const ArrayValue<T> otherT(other, dtype);
                return std::make_unique<ArrayValue<R>>(f(this->get_value(0), otherT.get_value(0)), output_dtype);
            } else if (this->get_size() == 1) { // left is a scalar
                const ArrayValue<T> otherT(other, dtype);
                arr.reserve(other->get_size());
                for (int i = 0; i < other->get_size(); i++)
                    arr.push_back(f(this->get_value(0), otherT.get_value(i)));
                return std::make_unique<ArrayValue<R>>(arr, otherT.get_shape(), output_dtype);
            } else if (other->get_size() == 1) { // right is a scalar
                const ArrayValue<T> otherT(other, dtype);
                arr.reserve(this->get_size());
                for (int i = 0; i < this->get_size(); i++)
                    arr.push_back(f(this->get_value(i), otherT.get_value(0)));
                return std::make_unique<ArrayValue<R>>(arr, this->get_shape(), output_dtype);
            } else { // both are arrays
                // test if shape match
                if (shape.size() != other->get_shape().size()) // Compare shape size
                    throw std::runtime_error("Arrays have incompatible shapes");
                for (int i = 0; i < shape.size(); i++) // Compare shape values
                    if (shape[i] != other->get_shape()[i])
                        throw std::runtime_error("Arrays have incompatible shapes");
                // apply operation
                const ArrayValue<T> otherT(other, dtype);
                arr.reserve(this->get_size());
                for (int i = 0; i < this->get_size(); i++)
                    arr.push_back(f(this->get_value(i), otherT.get_value(i)));
                return std::make_unique<ArrayValue<R>>(arr, this->get_shape(), output_dtype);
            }
        };

        /**
         * @brief Perform binary assignment operation with two array values
         *
         * @param other The right hand array value
         * @param f operation Function on self and other array value
         * @tparam R Output data type
         * @tparam Func Type of the operation function (is deduced)
         */
        template <typename R, typename Func> void operate_binary_equal(const BaseValue* other, Func f) {
            if (this->get_size() == 1 && other->get_size() == 1) { // both are scalars
                const ArrayValue<T> otherT(other, dtype);
                value[0] = f(this->get_value(0), otherT.get_value(0));
            } else if (this->get_size() == 1) { // left is a scalar
                const ArrayValue<T> otherT(other, dtype);
                T val = this->get_value(0);
                value.clear();
                value.reserve(otherT.get_size());
                shape = otherT.get_shape();
                for (int i = 0; i < otherT.get_size(); i++)
                    value.push_back(f(val, otherT.get_value(i)));
            } else if (other->get_size() == 1) { // right is a scalar
                const ArrayValue<T> otherT(other, dtype);
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
                const ArrayValue<T> otherT(other, dtype);
                for (int i = 0; i < this->get_size(); i++)
                    value[i] = f(this->get_value(i), otherT.get_value(i));
            }
        };

        template <typename U, typename R, typename Func>
        std::unique_ptr<ArrayValue<R>> operate_ternary(const BaseValue* other1, const BaseValue* other2, Func f) const {
            // test if shape match
            if (shape.size() != other1->get_shape().size() ||
                shape.size() != other2->get_shape().size()) // Compare shape size
                throw std::runtime_error("Arrays have incompatible shapes");
            for (int i = 0; i < shape.size(); i++) { // Compare shape values
                if (shape[i] != other1->get_shape()[i])
                    throw std::runtime_error("First other has incompatible shape");
                if (shape[i] != other2->get_shape()[i])
                    throw std::runtime_error("Second other has incompatible shape");
            }
            // apply operation
            const ArrayValue<U> other1T(other1, dtype);
            const ArrayValue<R> other2T(other2, dtype);
            std::vector<R> arr(value.size());
            for (int i = 0; i < value.size(); i++)
                arr[i] = f(other1T.get_value(i), value[i], other2T.value[i]);
            return std::make_unique<ArrayValue<R>>(arr, this->shape, dtype);
        };

      public:
        BaseValue::PointerType compare_equal(const BaseValue* other) const override {
            return operate_binary<uint8_t>(other, [](T a, T b) { return a == b; }, core::DataType::Boolean);
        };
        BaseValue::PointerType compare_not_equal(const BaseValue* other) const override {
            return operate_binary<uint8_t>(other, [](T a, T b) { return a != b; }, core::DataType::Boolean);
        };
        BaseValue::PointerType compare_less(const BaseValue* other) const override {
            return operate_binary<uint8_t>(other, [](T a, T b) { return a < b; }, core::DataType::Boolean);
        };
        BaseValue::PointerType compare_greater(const BaseValue* other) const override {
            return operate_binary<uint8_t>(other, [](T a, T b) { return a > b; }, core::DataType::Boolean);
        };
        BaseValue::PointerType compare_less_equal(const BaseValue* other) const override {
            return operate_binary<uint8_t>(other, [](T a, T b) { return a <= b; }, core::DataType::Boolean);
        };
        BaseValue::PointerType compare_greater_equal(const BaseValue* other) const override {
            return operate_binary<uint8_t>(other, [](T a, T b) { return a >= b; }, core::DataType::Boolean);
        };
        /*
         * Array slicing
         */
        BaseValue::PointerType slice_value(const Array::RangeType& slice) {
            if (slice.size() != this->shape.size())
                throw std::runtime_error(
                    "Array slice size does not correspond with array shape: " + std::to_string(slice.size()) +
                    "!=" + std::to_string(this->shape.size())
                );
            // calculate new shape and size
            Array::ShapeType new_shape;
            new_shape.reserve(this->shape.size());
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
                return std::make_unique<ArrayValue<T>>(new_value, new_shape, dtype);
            else
                return std::make_unique<ArrayValue<T>>(new_value[0], dtype);
        };
        BaseValue::PointerType where(const BaseValue* condition, const BaseValue* other) const override {
            return operate_ternary<bool, T>(condition, other, [](bool c, T a, T b) { return c ? a : b; });
        };
    };

} // namespace snt::val

#include "values_number.h"
#include "values_string.h"

#endif // VAL_VALUES_ARRAY_H
