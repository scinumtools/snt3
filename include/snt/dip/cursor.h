#ifndef DIP_CURSOR_H
#define DIP_CURSOR_H

#include <snt/dip/environment.h>
#include <snt/dip/nodes/path.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace snt::dip {

    class Cursor {
      protected:
        const Environment* env_;
        std::string path_;

      public:
        explicit Cursor(const Environment* env, std::string_view path = "") : env_(env), path_(path) {}

        Cursor get_group(std::string_view path) const;

        std::vector<Cursor> get_list(std::string_view path) const;

        std::unordered_map<std::string, Cursor> get_map(std::string_view path) const;

        const std::string& get_path() const;

        val::BaseValue::PointerType get_value() const { return env_->request_value("?" + path_); };

        val::Array::ShapeType get_shape() const;

        template <typename T> T to_scalar(size_t index = 0) const {
            val::BaseValue::PointerType value = env_->request_value("?" + path_);
            val::ArrayValue<T>* val = dynamic_cast<val::ArrayValue<T>*>(value.get());
            return val->get_value(index);
        };

        template <typename T> std::vector<T> to_vector() const {
            val::BaseValue::PointerType value = env_->request_value("?" + path_);
            val::ArrayValue<T>* val = dynamic_cast<val::ArrayValue<T>*>(value.get());
            return val->get_values();
        };

        const std::string to_string() const;
    };

} // namespace snt::dip

#endif // DIP_CURSOR_H
