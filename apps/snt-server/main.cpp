#include "httplib.h"
#include "settings.h"

#include <iostream>

int main(int argc, char* argv[]) {

    std::cout << "Starting SNT REST-API server on: http://localhost:" << SERVER_PORT << "/" << '\n';

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(
            R"(SNT REST-API Server

Example of use:
http://localhost:)" +
                std::to_string(SERVER_PORT) + R"(/snt/puq/

)",
            "text/plain"
        );
    });

    svr.listen(std::string(SERVER_ADDRESS), SERVER_PORT);
}
