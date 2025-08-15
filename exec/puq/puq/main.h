#ifndef MAIN_H
#define MAIN_H

#include <exception>
#include <iomanip>
#include <iostream>

#include "puq/calc/calc.h"
#include "puq/nostd/nostd.h"
#include "puq/quantity.h"
#include "puq/systems/systems.h"

void display_info(const std::string& expr);
void display_lists(std::deque<std::string>& convert);

#endif // MAIN_H
