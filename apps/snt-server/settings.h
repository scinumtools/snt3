#ifndef SNT_SERVER_SETTINGS_H
#define SNT_SERVER_SETTINGS_H

#include <string_view>

constexpr std::string_view SERVER_ADDRESS = "127.0.0.1"; ///< Default REST bind address, limited to loopback.
constexpr int SERVER_PORT = 8080;                        ///< Default REST listening port.

#endif // SNT_SERVER_SETTINGS_H
