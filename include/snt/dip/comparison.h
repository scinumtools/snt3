#ifndef SNT_DIP_COMPARISON_H
#define SNT_DIP_COMPARISON_H

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace snt::dip {
    class Environment;

    enum class ComparisonScope { Effective, Full };
    enum class DifferenceKind { Added, Removed, Changed };

    struct ComparisonOptions {
        ComparisonScope scope = ComparisonScope::Effective;
        std::size_t max_array_examples = 3;
    };

    /** One changed value or, in full scope, one changed manifest entry. */
    struct Difference {
        std::string path;
        std::string category; ///< value, source, trace, or schema.
        DifferenceKind kind;
        std::vector<std::string> fields;
        std::string before;
        std::string after;
        std::size_t changed_elements = 0;
        std::vector<std::size_t> example_indices; ///< Flat, zero-based array indices.
    };

    struct ComparisonResult {
        ComparisonScope scope = ComparisonScope::Effective;
        std::size_t added = 0;
        std::size_t removed = 0;
        std::size_t changed = 0;
        std::vector<Difference> differences; ///< Sorted by category and path.

        bool equal() const { return differences.empty(); }
    };

    /** Compare loaded evaluated environments without modifying them. */
    ComparisonResult compare(
        const Environment& before, const Environment& after, const ComparisonOptions& options = {}
    );

    /** Load and compare two DIPH5 snapshots. */
    ComparisonResult compare_diph5(
        const std::filesystem::path& before, const std::filesystem::path& after, const ComparisonOptions& options = {}
    );

    /** Render a bounded plain-text summary, shared by the language interfaces. */
    std::string render_comparison(const ComparisonResult& result, std::size_t max_details = 50);

    /** An owned comparison that can be inspected and rendered repeatedly.
     * File construction loads and compares both DIPH5 snapshots once.
     */
    class Comparison {
      public:
        Comparison(const Environment& before, const Environment& after, const ComparisonOptions& options = {});
        Comparison(const std::filesystem::path& before, const std::filesystem::path& after,
                   const ComparisonOptions& options = {});
        explicit Comparison(ComparisonResult result);

        const ComparisonResult& result() const { return result_; }
        bool equal() const { return result_.equal(); }
        std::string render(std::size_t max_details = 50) const;

      private:
        ComparisonResult result_;
    };
} // namespace snt::dip

#endif // SNT_DIP_COMPARISON_H
