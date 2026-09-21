#include "server.h"

#include "snt/api/puq_convert.h"
#include "snt/api/puq_eval.h"
#include "snt/api/puq_info.h"
#include "snt/api/puq_list.h"

namespace snt::server {

    namespace {

        void configure_puq(const httplib::Request& request, api::PUQEval& command) {
            if (request.has_param("input_system"))
                command.argument_input_system(request.get_param_value("input_system"));
            if (request.has_param("output_system"))
                command.argument_output_system(request.get_param_value("output_system"));
            if (request.has_param("output_units"))
                command.argument_output_units(request.get_param_value("output_units"));
            if (request.has_param("output_quantity"))
                command.argument_output_quantity(request.get_param_value("output_quantity"));
        }

        void configure_puq(const httplib::Request& request, api::PUQConvert& command) {
            if (request.has_param("input_system"))
                command.argument_input_system(request.get_param_value("input_system"));
            if (request.has_param("output_system"))
                command.argument_output_system(request.get_param_value("output_system"));
            if (request.has_param("output_quantity"))
                command.argument_output_quantity(request.get_param_value("output_quantity"));
        }

    } // namespace

    void register_puq_routes(httplib::Server& server) {
        server.Get("/snt/puq/eval", [](const httplib::Request& request, httplib::Response& response) {
            handle_json(response, [&] {
                api::PUQEval command(required_param(request, "expression"));
                configure_puq(request, command);
                return command.execute();
            });
        });

        server.Get("/snt/puq/convert", [](const httplib::Request& request, httplib::Response& response) {
            handle_json(response, [&] {
                api::PUQConvert command(required_param(request, "expression"), required_param(request, "output_units"));
                configure_puq(request, command);
                return command.execute();
            });
        });

        server.Get("/snt/puq/info", [](const httplib::Request& request, httplib::Response& response) {
            handle_json(response, [&] {
                api::PUQInfo command(required_param(request, "expression"));
                if (request.has_param("input_system"))
                    command.argument_input_system(request.get_param_value("input_system"));
                return command.execute();
            });
        });

        server.Get("/snt/puq/list", [](const httplib::Request& request, httplib::Response& response) {
            handle_json(response, [&] {
                api::PUQList command(request.has_param("list") ? request.get_param_value("list") : "");
                if (request.has_param("system"))
                    command.argument_system(request.get_param_value("system"));
                return command.execute();
            });
        });
    }

} // namespace snt::server
