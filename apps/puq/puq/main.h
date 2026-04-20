#ifndef MAIN_H
#define MAIN_H

#include <exception>
#include <iomanip>
#include <iostream>
#include <snt/puq/quantity.h>
#include <snt/puq/util/data_table.h>
#include <snt/puq/base_units.h>
#include <snt/puq/dimensions.h>
#include <snt/puq/measurement.h>

using namespace snt;

void display_info(const std::string& expr);
void display_lists(std::deque<std::string>& convert);

#endif // MAIN_H
