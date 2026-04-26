#ifndef APPS_SNT_PUQ_MAIN_H
#define APPS_SNT_PUQ_MAIN_H

#include "../argparser.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <snt/puq/base_units.h>
#include <snt/puq/dimensions.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/util/data_table.h>

namespace snt::cli {
  
  void puq_main(ArgParser& argpar);

}
  
#endif // APPS_SNT_PUQ_MAIN_H
