#ifndef SNT_API_DIP_COMPARE_H
#define SNT_API_DIP_COMPARE_H

#include <filesystem>
#include <snt/dip/comparison.h>
#include <string>
#include <utility>

namespace snt::api {
    /** Render a DIP comparison as bounded, deterministic plain text. */
    std::string render_dip_comparison(const dip::ComparisonResult& result, std::size_t max_details = 50);

    /** Compare two DIPH5 files and present the result for application interfaces. */
    class DIPCompare {
      public:
        DIPCompare(std::filesystem::path before, std::filesystem::path after)
            : before_(std::move(before)), after_(std::move(after)) {}

        void set_options(dip::ComparisonOptions options) { options_ = options; }
        void set_max_details(std::size_t count) { max_details_ = count; }
        dip::ComparisonResult compare() const;
        std::string execute() const;

      private:
        std::filesystem::path before_;
        std::filesystem::path after_;
        dip::ComparisonOptions options_;
        std::size_t max_details_ = 50;
    };
} // namespace snt::api

#endif // SNT_API_DIP_COMPARE_H
