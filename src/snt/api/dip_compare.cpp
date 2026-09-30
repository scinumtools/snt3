#include <snt/api/dip_compare.h>

namespace snt::api {
    std::string render_dip_comparison(const dip::ComparisonResult& result, std::size_t max_details) {
        return dip::render_comparison(result, max_details);
    }

    dip::ComparisonResult DIPCompare::compare() const {
        return dip::compare_diph5(before_, after_, options_);
    }

    std::string DIPCompare::execute() const {
        return render_dip_comparison(compare(), max_details_);
    }
} // namespace snt::api
