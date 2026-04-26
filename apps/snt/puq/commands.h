#ifndef APPS_SNT_PUQ_COMMANDS_H
#define APPS_SNT_PUQ_COMMANDS_H

#include <snt/puq/systems/unit_system.h>
#include <deque>
#include <string>

namespace snt::cli {

  void change_system(puq::UnitSystem& us, std::deque<std::string>& arguments);
  void command_solve(std::deque<std::string>& arguments);
  void command_convert(std::deque<std::string>& arguments);
  
}

#endif // APPS_SNT_PUQ_COMMANDS_H
