#include "hub.h"

#include <iostream>

int module_hub(int argc, char* argv[]) {
    try {
        return snt::hub::command(argc, argv);
    } catch (const snt::hub::Error& error) {
        std::cerr << "snt hub: " << error.what() << '\n';
        return error.code();
    } catch (const std::exception& error) {
        std::cerr << "snt hub: " << error.what() << '\n';
        return 2;
    }
}
