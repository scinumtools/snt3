#ifndef SNT_C_DIP_INTERNAL_H
#define SNT_C_DIP_INTERNAL_H

#include "snt/c/dip.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>

#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>

struct snt_dip {
    snt::dip::DIP parser;
    snt::dip::Environment env;
    bool parsed = false;
};

struct snt_dip_node {
    const snt_dip* owner;
    std::string path;
};

namespace snt::c::dip_detail {

    inline thread_local std::string last_error;

    inline int fail(snt_dip_error* error, const std::exception& exception) {
        last_error = exception.what();
        if (error) {
            error->code = 1;
            error->message = last_error.c_str();
        }
        return 1;
    }

    inline void ok(snt_dip_error* error) {
        if (error) {
            error->code = 0;
            error->message = nullptr;
        }
    }

    inline snt::dip::Cursor cursor(const snt_dip_node* node) {
        if (!node || !node->owner || !node->owner->parsed)
            throw std::invalid_argument("valid node and parsed environment are required");
        return snt::dip::Cursor(&node->owner->env, node->path);
    }

    inline int copy_string(
        const std::string& value, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
    ) {
        if (!required)
            throw std::invalid_argument("required-size output is required");
        *required = value.size() + 1;
        if (!buffer && capacity == 0) {
            ok(error);
            return 0;
        }
        if (!buffer || capacity < *required)
            throw std::invalid_argument("output buffer is too small");
        std::memcpy(buffer, value.c_str(), *required);
        ok(error);
        return 0;
    }

    template <typename F> int node_call(snt_dip_error* error, F&& function) {
        try {
            function();
            ok(error);
            return 0;
        } catch (const std::exception& exception) {
            return fail(error, exception);
        }
    }

} // namespace snt::c::dip_detail

#endif
