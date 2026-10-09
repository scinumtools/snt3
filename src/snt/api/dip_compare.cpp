#include <snt/api/dip_compare.h>

namespace snt::api {
    std::string render_dip_comparison(const dip::ComparisonResult& result, std::size_t max_details) {
        return dip::render_comparison(result, max_details);
    }

    const dip::Comparison& DIPCompare::comparison() const {
        if (!comparison_) comparison_.emplace(before_, after_, options_);
        return *comparison_;
    }

    dip::ComparisonResult DIPCompare::compare() const {
        return comparison().result();
    }

    std::string DIPCompare::execute() const {
        return comparison().render(max_details_);
    }
} // namespace snt::api
