#ifndef VAL_VALUES_ARRAY_H
#define VAL_VALUES_ARRAY_H

#include <snt/val/value_base.h>
#include <cstddef>
#include <iterator>
#include <memory>
#include <new>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>

namespace snt::val {

    // here we need a forward declaration
    template <typename T> class BaseArrayValue;
    template <typename T> class ArrayValue;

    /** Allocator for array storage whose elements are assigned before they are read.
     *
     * Default construction leaves scalar elements uninitialized. Array operation
     * kernels write each output element before exposing the resulting value.
     */
    template <typename T> class ArrayStorageAllocator {
      public:
        using value_type = T;
        using is_always_equal = std::true_type;

        ArrayStorageAllocator() noexcept = default;
        template <typename U> ArrayStorageAllocator(const ArrayStorageAllocator<U>&) noexcept {}

        T* allocate(const std::size_t count) { return std::allocator<T>{}.allocate(count); }
        void deallocate(T* pointer, const std::size_t count) noexcept {
            std::allocator<T>{}.deallocate(pointer, count);
        }

        template <typename U, typename... Args> void construct(U* pointer, Args&&... args) {
            if constexpr (sizeof...(Args) == 0)
                ::new (static_cast<void*>(pointer)) U;
            else
                ::new (static_cast<void*>(pointer)) U(std::forward<Args>(args)...);
        }

        template <typename U> void destroy(U* pointer) noexcept { pointer->~U(); }

        template <typename U> struct rebind { using other = ArrayStorageAllocator<U>; };
    };

    template <typename T, typename U>
    bool operator==(const ArrayStorageAllocator<T>&, const ArrayStorageAllocator<U>&) noexcept {
        return true;
    }
    template <typename T, typename U>
    bool operator!=(const ArrayStorageAllocator<T>&, const ArrayStorageAllocator<U>&) noexcept {
        return false;
    }

    template <typename T> using ArrayStorage = std::vector<T, ArrayStorageAllocator<T>>;

    /** Read-only view used as the input to a lazy elementwise expression. */
    template <typename T> class ArrayExpressionView {
      public:
        using value_type = T;

      private:
        const T* data;
        std::size_t size;
        Array::ShapeType shape;

      public:
        ArrayExpressionView(const T* values, std::size_t count, const Array::ShapeType& dimensions)
            : data(values), size(count), shape(dimensions) {}

        /** Read one source element. */
        T operator[](std::size_t index) const { return data[index]; }
        /** Number of source elements. */
        std::size_t get_size() const { return size; }
        /** Shape retained by the expression. */
        const Array::ShapeType& get_shape() const { return shape; }
    };

    /** Elementwise expression that maps each input value without allocating an intermediate array. */
    template <typename Input, typename Transform> class MappedArrayExpression {
      public:
        using value_type = std::decay_t<std::invoke_result_t<Transform, typename Input::value_type>>;

      private:
        Input input;
        Transform transform;

      public:
        MappedArrayExpression(Input source, Transform operation)
            : input(std::move(source)), transform(std::move(operation)) {}

        /** Evaluate the mapped value for one element. */
        value_type operator[](std::size_t index) const { return transform(input[index]); }
        /** Number of elements in the expression. */
        std::size_t get_size() const { return input.get_size(); }
        /** Shape retained by the expression. */
        const Array::ShapeType& get_shape() const { return input.get_shape(); }
    };

    /** Create a read-only expression over existing array data. */
    template <typename T>
    ArrayExpressionView<T> array_expression(const T* data, std::size_t size, const Array::ShapeType& shape) {
        return ArrayExpressionView<T>(data, size, shape);
    }

    /** Append one elementwise operation to an expression pipeline. */
    template <typename Input, typename Transform>
    auto map_array_expression(Input input, Transform transform) {
        return MappedArrayExpression<Input, Transform>(std::move(input), std::move(transform));
    }

    /** Evaluate a composed elementwise expression into one owned output buffer. */
    template <typename Expression>
    ArrayStorage<typename Expression::value_type> evaluate_array_expression(const Expression& expression) {
        ArrayStorage<typename Expression::value_type> values(expression.get_size());
        for (std::size_t i = 0; i < expression.get_size(); ++i)
            values[i] = expression[i];
        return values;
    }

    /**
     * Base class for all array values
     *
     * @tparam T Data type of a values
     */
    /** Typed array storage and arithmetic implementation. */
    template <typename T> class BaseArrayValue : public BaseValue {
        // We explicitely forbit these data types, to make sure only the standard data types are used
        static_assert(!std::is_same_v<T, bool>, "BaseArrayValue<bool> is not allowed");
        static_assert(!std::is_same_v<T, char>, "BaseArrayValue<char> is not allowed");

      protected:
        ArrayStorage<T> value; ///< Internal data container that holds flattened array values

      public:
        /**
         * Base array value constructor from a scalar value
         *
         * @param val Scalar value
         * @param dtype Datatype flag
         */
        BaseArrayValue(const T& val, const core::DataType dtype) : value({val}), BaseValue(dtype, {1}) {};

        /**
         * Base array value constructor from a flattened vector of values and a shape
         *
         * @param arr Vector of values
         * @param sh Array shape
         * @param dtype Datatype flag
         */
        BaseArrayValue(const std::vector<T>& arr, const Array::ShapeType& sh, const core::DataType dtype)
            : value(arr.begin(), arr.end()), BaseValue(dtype, sh) {};

        /**
         * Take ownership of an already allocated value vector.
         *
         * @param arr Owned flattened values to move into this array.
         * @param sh Array shape.
         * @param dtype Datatype flag.
         */
        BaseArrayValue(std::vector<T>&& arr, const Array::ShapeType& sh, const core::DataType dtype)
            : value(std::make_move_iterator(arr.begin()), std::make_move_iterator(arr.end())), BaseValue(dtype, sh) {};

        /** Copy from storage produced by an array operation.
         * @param arr Array values to copy.
         * @param sh Array shape.
         * @param dtype Data type flag.
         */
        BaseArrayValue(const ArrayStorage<T>& arr, const Array::ShapeType& sh, const core::DataType dtype)
            : value(arr), BaseValue(dtype, sh) {};

        /** Take ownership of array-operation output storage without copying it.
         * @param arr Internal storage produced by an array operation.
         * @param sh Array shape.
         * @param dtype Data type flag.
         */
        BaseArrayValue(ArrayStorage<T>&& arr, const Array::ShapeType& sh, const core::DataType dtype)
            : value(std::move(arr)), BaseValue(dtype, sh) {};

        /**
         * Base array value constructor from a BaseValue pointer
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
                this->value.assign(otherT->value.begin(), otherT->value.end());
            } else
                throw val::ArrayException(
                    "Invalid array initialization",
                    "The array value could not be initialized from the given value.",
                    "Provide a value with a compatible array type or a type that can be converted to the requested "
                    "data type.",
                    __FILE__,
                    __LINE__
                );
        };

        /**
         * Print object as a string
         */
        void print() override { std::cout << to_string() << std::endl; };

        /**
         * Get copy of the array values
         *
         * @return Flattened array of values
         */
        std::vector<T> get_values() const { return std::vector<T>(value.begin(), value.end()); };

        /**
         * Get copy of a particular value
         *
         * @param index Index of a value
         * @return Single value
         */
        T get_value(const size_t index) const { return value.at(index); };

        /**
         * Get pointer to the internal data container
         *
         * @return Pointer to the value data
         */
        T* get_data() { return value.data(); };

        /**
         * Get read-only access to the internal data container.
         *
         * @return Const pointer to the flattened array values.
         */
        const T* get_data() const { return value.data(); };

        /**
         * Get size of an array
         *
         * @return Size of the flattened array value
         */
        size_t get_size() const override { return value.size(); };

        /**
         * Print value into a string stream
         *
         * @param os String stream
         * @param val Value object
         */
        friend std::ostream& operator<<(std::ostream& os, const ArrayValue<T>& val) { return os << val.to_string(); };

        /**
         * Return string representation of this array object
         *
         * @param format Object with an output string format
         * @return String representation
         */
        std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override {
            return core::array_to_string(get_values(), shape, dtype, format);
        };

        /*
         * Operations
         */
      protected:
        /**
         * Perform unary operation on a array value
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
            ArrayStorage<R> arr(value.size());
            for (int i = 0; i < value.size(); i++)
                arr[i] = static_cast<R>(f(value[i]));
            return std::make_unique<ArrayValue<R>>(std::move(arr), this->shape, output_dtype);
        };

        /**
         * Perform unary assignment operation on a array value
         *
         * @param f operation Function on self and other array value
         * @tparam R Output data type
         * @tparam Func Type of the operation function (is deduced)
         */
        template <typename R, typename Func> void operate_unary_equal(Func f) {
            // Keep the data pointer and length outside the loop so the compiler
            // can optimize the contiguous in-place traversal.
            T* values = value.data();
            const std::size_t size = value.size();
            for (std::size_t i = 0; i < size; ++i)
                values[i] = f(values[i]);
        };

        /**
         * Perform binary operation on two array values
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
            ArrayStorage<R> arr;
            const std::size_t left_size = this->get_size();
            const std::size_t right_size = other->get_size();
            // Reuse an already compatible array as a read-only view. Constructing
            // ArrayValue<T> from BaseValue copies every element, so only convert
            // when the input storage type actually differs.
            std::unique_ptr<ArrayValue<T>> converted_other;
            const auto* otherT = other->get_dtype() == dtype ? dynamic_cast<const ArrayValue<T>*>(other) : nullptr;
            if (!otherT) {
                converted_other = std::make_unique<ArrayValue<T>>(other, dtype);
                otherT = converted_other.get();
            }
            if (left_size == 1 && right_size == 1) { // both are scalars
                return std::make_unique<ArrayValue<R>>(f(value[0], otherT->get_data()[0]), output_dtype);
            } else if (left_size == 1) { // left is a scalar
                arr.resize(right_size);
                const T left_value = value[0];
                const T* right_values = otherT->get_data();
                for (std::size_t i = 0; i < right_size; ++i)
                    arr[i] = f(left_value, right_values[i]);
                return std::make_unique<ArrayValue<R>>(std::move(arr), otherT->get_shape(), output_dtype);
            } else if (right_size == 1) { // right is a scalar
                arr.resize(left_size);
                const T right_value = otherT->get_data()[0];
                for (std::size_t i = 0; i < left_size; ++i)
                    arr[i] = f(value[i], right_value);
                return std::make_unique<ArrayValue<R>>(std::move(arr), this->get_shape(), output_dtype);
            } else { // both are arrays
                // test if shape match
                if (shape.size() != other->get_shape().size()) // Compare shape size
                    throw val::ArrayException(
                        "Dimension mismatch",
                        "The arrays have different numbers of dimensions: `" + std::to_string(shape.size()) + "` != `" +
                            std::to_string(other->get_shape().size()) + "`.",
                        "Use arrays with the same number of dimensions.",
                        __FILE__,
                        __LINE__
                    );
                for (int i = 0; i < shape.size(); i++) // Compare shape values
                    if (shape[i] != other->get_shape()[i])
                        throw val::ArrayException(
                            "Shape mismatch",
                            "The arrays have different sizes in dimension `" + std::to_string(i) + "`: `" +
                                std::to_string(shape[i]) + "` != `" + std::to_string(other->get_shape()[i]) + "`.",
                            "Use arrays with the same size in every dimension.",
                            __FILE__,
                            __LINE__
                        );
                // apply operation
                const T* left_values = value.data();
                const T* right_values = otherT->get_data();
                arr.resize(left_size);
                for (std::size_t i = 0; i < left_size; ++i)
                    arr[i] = f(left_values[i], right_values[i]);
                return std::make_unique<ArrayValue<R>>(std::move(arr), this->get_shape(), output_dtype);
            }
        };

        /**
         * Perform binary assignment operation with two array values
         *
         * @param other The right hand array value
         * @param f operation Function on self and other array value
         * @tparam R Output data type
         * @tparam Func Type of the operation function (is deduced)
         */
        template <typename R, typename Func> void operate_binary_equal(const BaseValue* other, Func f) {
            const std::size_t left_size = value.size();
            const std::size_t right_size = other->get_size();

            // Keep compatible input arrays as read-only views. Conversion is
            // still performed when the value type or dtype differs.
            std::unique_ptr<ArrayValue<T>> converted_other;
            const auto* otherT = other->get_dtype() == dtype ? dynamic_cast<const ArrayValue<T>*>(other) : nullptr;
            if (!otherT) {
                converted_other = std::make_unique<ArrayValue<T>>(other, dtype);
                otherT = converted_other.get();
            }
            const T* right_values = otherT->get_data();

            if (left_size == 1 && right_size == 1) { // both are scalars
                value[0] = f(value[0], right_values[0]);
            } else if (left_size == 1) { // left scalar expands to the right array
                const T left_value = value[0];
                ArrayStorage<T> expanded(right_size);
                for (std::size_t i = 0; i < right_size; ++i)
                    expanded[i] = f(left_value, right_values[i]);
                value = std::move(expanded);
                shape = otherT->get_shape();
            } else if (right_size == 1) { // right scalar broadcasts across the left array
                const T right_value = right_values[0];
                T* left_values = value.data();
                for (std::size_t i = 0; i < left_size; ++i)
                    left_values[i] = f(left_values[i], right_value);
            } else { // both are arrays
                // test if shape match
                if (shape.size() != other->get_shape().size()) // Compare shape size
                    throw val::ArrayException(
                        "Dimension mismatch",
                        "The arrays have different numbers of dimensions: `" + std::to_string(shape.size()) + "` != `" +
                            std::to_string(other->get_shape().size()) + "`.",
                        "Use arrays with the same number of dimensions.",
                        __FILE__,
                        __LINE__
                    );
                for (int i = 0; i < shape.size(); i++) // Compare shape values
                    if (shape[i] != other->get_shape()[i])
                        throw val::ArrayException(
                            "Shape mismatch",
                            "The arrays have different sizes in dimension `" + std::to_string(i) + "`: `" +
                                std::to_string(shape[i]) + "` != `" + std::to_string(other->get_shape()[i]) + "`.",
                            "Use arrays with the same size in every dimension.",
                            __FILE__,
                            __LINE__
                        );
                // apply operation
                T* left_values = value.data();
                for (std::size_t i = 0; i < left_size; ++i)
                    left_values[i] = f(left_values[i], right_values[i]);
            }
        };

        /**
         * Perform ternary operation with three array values
         *
         * @param other1 The first ternary argument, usually a boolean condition result
         * @param other2 The third ternary argument, usually an alternative to this object
         * @param f operation Ternary function, usually a selection of the second and third argument based on the first
         * @param first_dtype DataType flag of the first ternary argument (needed for booleans)
         * @param third_dtype DataType flag of the third ternary argument (needed for booleans)
         * @tparam U Data type of the first ternary argument
         * @tparam R Data type of the second and third ternary argument
         * @tparam Func Type of the operation function (is deduced)
         */
        template <typename U, typename R, typename Func>
        std::unique_ptr<ArrayValue<R>> operate_ternary(
            const BaseValue* other1,
            const BaseValue* other2,
            Func f,
            core::DataType first_dtype = core::DataType::None,
            core::DataType third_dtype = core::DataType::None
        ) const {
            if (first_dtype == core::DataType::None)
                first_dtype = dtype;
            if (third_dtype == core::DataType::None)
                third_dtype = dtype;
            // test if shape match
            if (shape.size() != other1->get_shape().size())
                throw val::ArrayException(
                    "Dimension mismatch",
                    "The first other array has a different number of dimensions: `" + std::to_string(shape.size()) +
                        "` != `" + std::to_string(other1->get_shape().size()) + "`.",
                    "Use arrays with the same number of dimensions.",
                    __FILE__,
                    __LINE__
                );
            if (shape.size() != other2->get_shape().size())
                throw val::ArrayException(
                    "Dimension mismatch",
                    "The second other array has a different number of dimensions: `" + std::to_string(shape.size()) +
                        "` != `" + std::to_string(other2->get_shape().size()) + "`.",
                    "Use arrays with the same number of dimensions.",
                    __FILE__,
                    __LINE__
                );
            for (int i = 0; i < shape.size(); i++) { // Compare shape values
                if (shape[i] != other1->get_shape()[i])
                    throw val::ArrayException(
                        "Shape mismatch",
                        "The first other array has a different size in dimension `" + std::to_string(i) + "`: `" +
                            std::to_string(shape[i]) + "` != `" + std::to_string(other1->get_shape()[i]) + "`.",
                        "Use arrays with the same size in every dimension.",
                        __FILE__,
                        __LINE__
                    );
                if (shape[i] != other2->get_shape()[i])
                    throw val::ArrayException(
                        "Shape mismatch",
                        "The second other array has a different size in dimension `" + std::to_string(i) + "`: `" +
                            std::to_string(shape[i]) + "` != `" + std::to_string(other2->get_shape()[i]) + "`.",
                        "Use arrays with the same size in every dimension.",
                        __FILE__,
                        __LINE__
                    );
            }
            // apply operation
            const ArrayValue<U> other1T(other1, first_dtype);
            const ArrayValue<R> other2T(other2, third_dtype);
            ArrayStorage<R> arr(value.size());
            for (int i = 0; i < value.size(); i++)
                arr[i] = f(other1T.get_value(i), value[i], other2T.value[i]);
            return std::make_unique<ArrayValue<R>>(std::move(arr), this->shape, dtype);
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
        /** Create a value containing the selected array ranges.
         * @param slice Ranges selecting the requested elements.
         */
        BaseValue::PointerType slice_value(const Array::RangeType& slice) {
            if (slice.size() != this->shape.size())
                throw val::ArrayException(
                    "Dimension mismatch",
                    "The slice has `" + std::to_string(slice.size()) + "` dimensions, but the array has `" +
                        std::to_string(this->shape.size()) + "` dimensions.",
                    "Specify a slice with the same number of dimensions as the array.",
                    __FILE__,
                    __LINE__
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
                    if (coord[dim] < dmin || dmax < coord[dim])
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
                return std::make_unique<ArrayValue<T>>(std::move(new_value), new_shape, dtype);
            else
                return std::make_unique<ArrayValue<T>>(new_value[0], dtype);
        };

        BaseValue::PointerType where(const BaseValue* condition, const BaseValue* other) const override {
            return operate_ternary<uint8_t, T>(
                condition,
                other,
                [](uint8_t c, T a, T b) { return static_cast<bool>(c) ? a : b; },
                core::DataType::Boolean
            );
        };
    };

} // namespace snt::val

#include "values_number.h"
#include "values_string.h"

#endif // VAL_VALUES_ARRAY_H
