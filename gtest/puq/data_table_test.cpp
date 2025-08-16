#include <gtest/gtest.h>
#include <sstream>

#include "puq/util/data_table.h"

TEST(DataTable, TextOutput) {

  puq::DataTable tab({{"A", 5}, {"B", 7}, {"C", 10}});
  tab.append({"d", "e", "f"});
  tab.append({"g", "h", "i"});
  std::stringstream ss;
  ss << "A     B       C          \n";
  ss << "----- ------- ---------- \n";
  ss << "d     e       f          \n";
  ss << "g     h       i          \n";
  EXPECT_EQ(tab.to_string(), ss.str());
}

TEST(DataTable, JsonOutput) {

  puq::DataTable tab({{"A", 5}, {"B", 7}, {"C", 10}});
  tab.append({"d", "e", "f"});
  tab.append({"g", "h", "i"});
  std::stringstream ss;
  ss << "{";
  ss << "\"A\":[\"d\",\"g\"],";
  ss << "\"B\":[\"e\",\"h\"],";
  ss << "\"C\":[\"f\",\"i\"]";
  ss << "}";
  EXPECT_EQ(tab.to_json(), ss.str());
}
