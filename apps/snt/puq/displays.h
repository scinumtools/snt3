#ifndef APPS_SNT_PUQ_DISPLAYS_H
#define APPS_SNT_PUQ_DISPLAYS_H

#include <exception>
#include <iomanip>
#include <iostream>
#include <snt/puq/base_units.h>
#include <snt/puq/dimensions.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/util/data_table.h>

namespace snt::cli {
  
  void display_info(const std::string& expr);
  void display_lists(std::deque<std::string>& convert);

}
  
#endif // APPS_SNT_PUQ_DISPLAYS_H
