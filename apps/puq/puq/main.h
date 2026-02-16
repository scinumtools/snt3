#ifndef MAIN_H
#define MAIN_H

#include <snt/puq/value/unit_value.h>
#include <snt/puq/quantity.h>
#include <snt/puq/value/base_units.h>
#include <snt/puq/value/dimensions.h>
#include <snt/puq/util/data_table.h>

#include <exception>
#include <iomanip>
#include <iostream>

using namespace snt;

void display_info(const std::string& expr);
void display_lists(std::deque<std::string>& convert);

#endif // MAIN_H
