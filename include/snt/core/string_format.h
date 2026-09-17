#ifndef SNT_STRING_FORMAT_H
#define SNT_STRING_FORMAT_H

#include <snt/core/datatypes.h>
#include <snt/core/settings.h>

namespace snt::core {

    /** Formatting options shared by textual value and array renderers. */
    struct StringFormatType {
        char specifier = 'g';         ///< Format specifier: general, scientific (`e`), fixed (`f`), or decimal (`d`).
        int valuePrecision = 4;       ///< Number of significant digits used for values.
        int uncertaintyPrecision = 2; ///< Number of significant digits used for uncertainties.
        int thresholdScientific = 3;  ///< Exponent at which general notation switches to scientific notation.
        bool paddingZeros = false;    ///< Pad formatted output with zeroes instead of spaces.
        int paddingSize = 0;          ///< Minimum width of formatted output.
        bool stringQuotes = true;     ///< Surround string values with quotes and escape embedded quotes.
    };

    // Implementation for value only

    /** Format a scalar value according to @p format. */
    template <typename T>
    std::string number_to_string(const T& value, const StringFormatType& format = StringFormatType()) {
        std::stringstream ss;
        if (value == 0) {
            return "0";
        } else {
            ss << std::setprecision(format.valuePrecision);
            if (format.specifier == 'e') {
                ss << std::scientific << value;
            } else if (format.specifier == 'f') {
                ss << std::fixed << value;
            } else if (format.specifier == 'd') {
                ss << std::dec << value;
            } else if (format.specifier == 'g') {
                ss << std::defaultfloat;
                int exp_val = std::floor(std::log10(std::fabs(value))); // rounded exponent
                if (std::abs(exp_val) >= format.thresholdScientific && std::is_floating_point_v<T>) {
                    // if exponent is a floating point number larger or equal than thresholdScientific
                    T val_mag = value * std::pow(10, -exp_val); // numerical value without x10^ part
                    ss << val_mag;
                    if (exp_val)
                        ss << 'e' << exp_val;
                } else {
                    // in all other cases
                    ss << value;
                }
            }
        }
        if (format.paddingSize) {
            std::string str = ss.str();
            ss.str("");
            ss.clear();
            if (format.paddingZeros)
                ss << std::setfill('0');
            ss << std::setw(format.paddingSize) << str;
        }
        return ss.str();
    }

    /** Escape double quotes in a string when requested. */
    inline std::string escape_quotes(const std::string& value, bool escape) {
        if (escape) {
            std::string output;
            for (char c : value) {
                if (c == '"') {
                    output += "\\\""; // add \"
                } else {
                    output += c;
                }
            }
            return output;
        }
        return value;
    }

    template <typename T>
    std::string _array_to_string(
        const std::vector<T>& value,
        const std::vector<size_t>& shape,
        const DataType dtype,
        const StringFormatType& format,
        size_t& offset,
        size_t dim
    ) {
        std::ostringstream oss;
        if (value.size() > 1 || value.size() == 0)
            oss << SYMBOL_ARRAY_START;
        for (size_t i = 0; i < shape[dim]; i++) {
            if (i > 0)
                oss << SYMBOL_ARRAY_DELIMITER;
            if (dim + 1 < shape.size()) {
                oss << _array_to_string(value, shape, dtype, format, offset, dim + 1);
            } else {
                if constexpr (std::is_same_v<T, bool>) {
                    oss << (value[offset] ? KEYWORD_TRUE : KEYWORD_FALSE);
                } else if constexpr (std::is_same_v<T, char>) {
                    oss << value[offset];
                } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
                    if (dtype == core::DataType::Boolean) {
                        oss << (value[offset] ? KEYWORD_TRUE : KEYWORD_FALSE);
                    } else if (dtype == core::DataType::Character) {
                        oss << (char)value[offset];
                    } else {
                        oss << number_to_string(value[offset], format);
                    }
                } else if constexpr (std::is_same_v<T, std::string>) {
                    if (format.stringQuotes)
                        oss << '"';
                    if (format.paddingSize)
                        oss << std::setw(format.paddingSize) << escape_quotes(value[offset], format.stringQuotes);
                    else
                        oss << escape_quotes(value[offset], format.stringQuotes);
                    if (format.stringQuotes)
                        oss << '"';
                } else {
                    return "<unsupported_type>";
                }
                offset++;
            }
        }
        if (value.size() > 1 || value.size() == 0)
            oss << SYMBOL_ARRAY_END;
        return oss.str();
    }

    /** Format a multidimensional array of values according to its shape and type. */
    template <typename T>
    std::string array_to_string(
        const std::vector<T>& value,
        const std::vector<size_t>& shape,
        const DataType dtype,
        const StringFormatType& format = StringFormatType()
    ) {
        size_t offset = 0;
        return _array_to_string(value, shape, dtype, format, offset, 0);
    }

    // Implementation for value/uncertainty

    /** Format a value together with its uncertainty. */
    template <typename T>
    std::string number_to_string(
        const T& value, const T& uncertainty, const StringFormatType& format = StringFormatType()
    ) {
        std::stringstream ss;
        if (value == 0 && uncertainty == 0) {
            return "0";
        } else if (uncertainty == 0) {
            return number_to_string(value, format);
        } else {
            int exp_val = std::floor(std::log10(std::fabs(value))); // rounded exponent
            int exp_err = std::floor(std::log10(std::fabs(uncertainty)));
            int exp_diff = std::abs(exp_val - exp_err) - 1;
            ss << std::fixed;
            if (std::is_integral_v<T>) {
                int prec = (exp_err) ? format.uncertaintyPrecision - 1 : 0;
                int val_err = static_cast<int>(uncertainty * std::pow(10, -exp_err + prec));
                T val_mag = value * std::pow(10, -exp_err + prec); // numerical value without x10^ part
                ss << std::setprecision(exp_diff);
                ss << val_mag;
                ss << "(" << std::setprecision(format.uncertaintyPrecision) << val_err << ")";
                if (exp_err - prec)
                    ss << 'e' << exp_err - prec;
            } else if (std::is_floating_point_v<T>) {
                int val_err = static_cast<int>(
                    std::round(uncertainty * std::pow(10, -exp_err - 1 + format.uncertaintyPrecision))
                );
                T val_mag = value * std::pow(10, -exp_val); // numerical value without x10^ part
                ss << std::setprecision(exp_diff + format.uncertaintyPrecision);
                ss << val_mag;
                ss << "(" << std::setprecision(format.uncertaintyPrecision) << val_err << ")";
                if (exp_val)
                    ss << 'e' << exp_val;
            }
        }
        if (format.paddingSize) {
            std::string str = ss.str();
            ss.str("");
            ss.clear();
            if (format.paddingZeros)
                ss << std::setfill('0');
            ss << std::setw(format.paddingSize) << str;
        }
        return ss.str();
    }

    template <typename T>
    std::string _array_to_string(
        const std::vector<T>& value,
        const std::vector<T>& uncertainty,
        const std::vector<size_t>& shape,
        const StringFormatType& format,
        size_t& offset,
        size_t dim
    ) {
        std::ostringstream oss;
        if (value.size() > 1)
            oss << SYMBOL_ARRAY_START;
        for (size_t i = 0; i < shape[dim]; i++) {
            if (i > 0)
                oss << SYMBOL_ARRAY_DELIMITER;
            if (dim + 1 < shape.size()) {
                oss << _array_to_string(value, uncertainty, shape, format, offset, dim + 1);
            } else {
                if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
                    oss << number_to_string(value[offset], uncertainty[offset], format);
                } else {
                    return "<unsupported_type>";
                }
                offset++;
            }
        }
        if (value.size() > 1)
            oss << SYMBOL_ARRAY_END;
        return oss.str();
    }

    /** Format a multidimensional array of values and uncertainties. */
    template <typename T>
    std::string array_to_string(
        const std::vector<T>& value,
        const std::vector<T>& uncertainty,
        const std::vector<size_t>& shape,
        const StringFormatType& format = StringFormatType()
    ) {
        size_t offset = 0;
        return _array_to_string(value, uncertainty, shape, format, offset, 0);
    }

} // namespace snt::core

#endif // SNT_STRING_FORMAT_H
