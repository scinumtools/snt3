#include "snt/c/dip.h"

#include <cstring>
#include <memory>
#include <snt/dip/comparison.h>
#include <stdexcept>
#include <string>
#include <vector>

struct snt_dip_comparison {
    snt::dip::ComparisonResult result;
    std::vector<std::string> field_names;
};

namespace {
    thread_local std::string comparison_error;

    int fail(snt_dip_error* error, const std::exception& exception) {
        comparison_error = exception.what();
        if (error) {
            error->code = 1;
            error->message = comparison_error.c_str();
        }
        return 1;
    }

    void ok(snt_dip_error* error) {
        if (error) {
            error->code = 0;
            error->message = nullptr;
        }
    }
} // namespace

extern "C" int snt_dip_compare_files(
    const char* before,
    const char* after,
    snt_dip_compare_scope scope,
    size_t max_array_examples,
    snt_dip_comparison** result,
    snt_dip_error* error
) {
    try {
        if (!before || !after || !result)
            throw std::invalid_argument("before, after, and result are required");
        if (scope != SNT_DIP_COMPARE_EFFECTIVE && scope != SNT_DIP_COMPARE_FULL)
            throw std::invalid_argument("invalid comparison scope");
        *result = nullptr;
        snt::dip::ComparisonOptions options;
        options.scope =
            scope == SNT_DIP_COMPARE_FULL ? snt::dip::ComparisonScope::Full : snt::dip::ComparisonScope::Effective;
        options.max_array_examples = max_array_examples;
        auto owned = std::make_unique<snt_dip_comparison>();
        owned->result = snt::dip::compare_diph5(before, after, options);
        owned->field_names.reserve(owned->result.differences.size());
        for (const auto& difference : owned->result.differences) {
            std::string names;
            for (const auto& field : difference.fields) {
                if (!names.empty())
                    names += ',';
                names += field;
            }
            owned->field_names.push_back(std::move(names));
        }
        *result = owned.release();
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_comparison_summary(
    const snt_dip_comparison* result, size_t* added, size_t* removed, size_t* changed, snt_dip_error* error
) {
    try {
        if (!result || !added || !removed || !changed)
            throw std::invalid_argument("result and summary outputs are required");
        *added = result->result.added;
        *removed = result->result.removed;
        *changed = result->result.changed;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" size_t snt_dip_comparison_count(const snt_dip_comparison* result) {
    return result ? result->result.differences.size() : 0;
}

extern "C" int snt_dip_comparison_get(
    const snt_dip_comparison* result, size_t index, snt_dip_difference* output, snt_dip_error* error
) {
    try {
        if (!result || !output || index >= result->result.differences.size())
            throw std::invalid_argument("comparison, valid index, and output are required");
        const auto& difference = result->result.differences[index];
        *output = {
            difference.path.c_str(),
            difference.category.c_str(),
            difference.kind == snt::dip::DifferenceKind::Added     ? SNT_DIP_DIFFERENCE_ADDED
            : difference.kind == snt::dip::DifferenceKind::Removed ? SNT_DIP_DIFFERENCE_REMOVED
                                                                   : SNT_DIP_DIFFERENCE_CHANGED,
            result->field_names[index].c_str(),
            difference.before.c_str(),
            difference.after.c_str(),
            difference.changed_elements,
            difference.example_indices.data(),
            difference.example_indices.size()
        };
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_comparison_render_text(
    const snt_dip_comparison* result,
    size_t max_details,
    char* buffer,
    size_t capacity,
    size_t* required,
    snt_dip_error* error
) {
    try {
        if (!result || !required)
            throw std::invalid_argument("comparison and required size are required");
        const auto text = snt::dip::render_comparison(result->result, max_details);
        *required = text.size() + 1;
        if (buffer) {
            if (capacity < *required)
                throw std::invalid_argument("output buffer is too small");
            std::memcpy(buffer, text.c_str(), *required);
        } else if (capacity != 0) {
            throw std::invalid_argument("capacity must be zero when buffer is null");
        }
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" void snt_dip_comparison_free(snt_dip_comparison* result) {
    delete result;
}
