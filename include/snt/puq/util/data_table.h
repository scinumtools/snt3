#ifndef PUQ_DATA_TABLE_H
#define PUQ_DATA_TABLE_H

#include <iomanip>
#include <snt/puq/util/display_length.h>
#include <sstream>
#include <vector>
#include <utility>

namespace snt::puq {

    /** One row or column definition used by a formatted data table. */
    struct DataTableStruct {
        std::string title;
        int width;
    };

    /** Lightweight text table formatter used in diagnostics and output. */
    class DataTable {
        static const int padding = 1;
        std::vector<std::vector<std::string>> data;
        std::vector<DataTableStruct> settings;

      public:
        DataTable(std::vector<DataTableStruct> s) : settings(std::move(s)) {};

        /** Append an item to the collection.
         * @param columns Column names to append.
         */
        void append(std::vector<std::string> columns) { data.push_back(std::move(columns)); };

        std::string to_string() {
            std::stringstream ss;
            for (const auto& setting : settings) {
                ss << std::setfill(' ') << std::setw(setting.width) << std::left << setting.title
                   << std::string(padding, ' ');
            }
            ss << '\n';
            for (const auto& setting : settings) {
                if (setting.title.empty())
                    ss << std::string(setting.width + padding, ' ');
                else
                    ss << std::setfill(' ') << std::setw(setting.width) << std::left
                       << std::string(setting.width, '-') << std::string(padding, ' ');
            }
            ss << '\n';
            for (const auto& row : data) {
                for (size_t col = 0; col < row.size(); col++) {
                    size_t dwidth = row[col].size() - display_length(row[col]);
                    ss << std::setfill(' ') << std::setw(static_cast<int>(settings[col].width + dwidth)) << std::left << row[col]
                       << std::string(padding, ' ');
                }
                ss << '\n';
            }
            return ss.str();
        }

        std::string to_json() {
            std::stringstream ss;
            ss << "{";
            for (size_t col = 0; col < settings.size(); col++) {
                ss << '"' << settings[col].title << '"' << ":[";
                for (size_t row = 0; row < data.size(); row++) {
                    ss << '"' << data[row][col] << '"';
                    if (row < data.size() - 1)
                        ss << ",";
                }
                ss << "]";
                if (col < settings.size() - 1)
                    ss << ",";
            }
            ss << "}";
            return ss.str();
        }
    };

} // namespace snt::puq

#endif // PUQ_DATA_TABLE_H
