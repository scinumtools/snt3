#include <algorithm>
#include <snt/dip/comparison.h>
#include <sstream>

namespace snt::dip {
    namespace {
        std::string one_line(const std::string& value) {
            std::string result;
            for (const char c : value) {
                if (c == '\n')
                    result += "\\n";
                else if (c == '\r')
                    result += "\\r";
                else
                    result += c;
            }
            return result;
        }
    } // namespace

    std::string render_comparison(const ComparisonResult& result, std::size_t max_details) {
        std::ostringstream output;
        output << (result.scope == ComparisonScope::Full ? "Full" : "Effective") << " DIPH5 comparison\n";
        output << result.added << " added, " << result.removed << " removed, " << result.changed << " changed\n";
        if (result.equal())
            return output.str();
        output << '\n';
        const auto count = std::min(max_details, result.differences.size());
        for (std::size_t i = 0; i < count; ++i) {
            const auto& difference = result.differences[i];
            const char marker = difference.kind == DifferenceKind::Added     ? '+'
                                : difference.kind == DifferenceKind::Removed ? '-'
                                                                             : '~';
            output << marker << ' ';
            if (difference.category != "value")
                output << difference.category << ':';
            output << one_line(difference.path);
            if (!difference.fields.empty()) {
                output << " [";
                for (std::size_t j = 0; j < difference.fields.size(); ++j) {
                    if (j)
                        output << ", ";
                    output << difference.fields[j];
                }
                output << ']';
            }
            if (difference.kind == DifferenceKind::Added)
                output << " = " << one_line(difference.after);
            else if (difference.kind == DifferenceKind::Removed)
                output << " = " << one_line(difference.before);
            else if (difference.before != difference.after)
                output << ": " << one_line(difference.before) << " -> " << one_line(difference.after);
            if (difference.changed_elements) {
                output << "; " << difference.changed_elements << " elements differ";
                if (!difference.example_indices.empty()) {
                    output << "; first flat indices: ";
                    for (std::size_t j = 0; j < difference.example_indices.size(); ++j) {
                        if (j)
                            output << ", ";
                        output << '[' << difference.example_indices[j] << ']';
                    }
                }
            }
            output << '\n';
        }
        if (count < result.differences.size())
            output << "... " << result.differences.size() - count << " more differences\n";
        return output.str();
    }

    std::string Comparison::render(std::size_t max_details) const {
        return render_comparison(result_, max_details);
    }
} // namespace snt::dip
