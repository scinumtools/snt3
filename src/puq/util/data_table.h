#ifndef PUQ_DATA_TABLE_H
#define PUQ_DATA_TABLE_H

#include <iomanip>
#include "display_length.h"

namespace puq {

  struct DataTableStruct {
    std::string title;
    int width;
  };

  class DataTable {
    static const int padding = 1;
    std::vector<std::vector<std::string>> data;
    std::vector<DataTableStruct> settings;

  public:
    DataTable(std::vector<DataTableStruct> s) : settings(s) {};

    void append(std::vector<std::string> columns) {
      data.push_back(columns);
    };

    std::string to_string() {
      std::stringstream ss;
      for (size_t i = 0; i < settings.size(); i++) {
        ss << std::setfill(' ') << std::setw(settings[i].width) << std::left << settings[i].title << std::string(padding, ' ');
      }
      ss << std::endl;
      for (size_t i = 0; i < settings.size(); i++) {
        if (settings[i].title.size() == 0)
          ss << std::string(settings[i].width + padding, ' ');
        else
          ss << std::setfill(' ') << std::setw(settings[i].width) << std::left << std::string(settings[i].width, '-') << std::string(padding, ' ');
      }
      ss << std::endl;
      for (size_t row = 0; row < data.size(); row++) {
        for (size_t col = 0; col < data[row].size(); col++) {
          size_t dwidth = data[row][col].size() - display_length(data[row][col]);
          ss << std::setfill(' ') << std::setw(settings[col].width + dwidth) << std::left << data[row][col] << std::string(padding, ' ');
        }
        ss << std::endl;
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

} // namespace puq

#endif // PUQ_DATA_TABLE_H
