#include "server.h"

#include <snt/core/string_format.h>
#include <snt/dip/dip.h>

#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace snt::server {

    namespace {

        class PublishedEnvironments {
          private:
            std::map<std::string, dip::Environment> environments;

          public:
            explicit PublishedEnvironments(const std::vector<PublishedInput>& inputs) {
                for (const auto& input : inputs) {
                    if (!std::filesystem::is_regular_file(input.file))
                        throw std::invalid_argument(
                            "Published environment input is not a regular file: " + input.file.string()
                        );
                    dip::Environment environment;
                    if (input.kind == PublishedInputKind::Project) {
                        dip::DIP dip;
                        dip.add_project(input.file);
                        environment = dip.parse();
                    } else {
                        environment.load(input.file);
                    }
                    if (!environments.emplace(input.name, std::move(environment)).second)
                        throw std::invalid_argument("Published environment name is duplicated: " + input.name);
                }
            }

            const dip::Environment& get(const std::string& name) const {
                const auto environment = environments.find(name);
                if (environment == environments.end())
                    throw std::invalid_argument("Unknown published environment: " + name);
                return environment->second;
            }

            std::string list() const {
                std::ostringstream output;
                output << R"({"environments":[)";
                bool first = true;
                for (const auto& [name, _] : environments) {
                    if (!first)
                        output << ',';
                    output << '"' << name << '"';
                    first = false;
                }
                return output.str() + "]}\n";
            }
        };

        std::string render_environment(
            const dip::Environment& environment, const httplib::Request& request
        ) {
            const std::string path = request.has_param("request") ? request.get_param_value("request") : "";
            const auto tags = request.has_param("tags") ? split_tags(request.get_param_value("tags"))
                                                        : std::vector<std::string>{};
            const auto nodes = environment.request_group(
                path.empty() ? "?" : "?" + (path.front() == '?' ? path.substr(1) : path),
                dip::RequestType::Reference,
                tags
            );
            if (!optional_flag(request, "value")) {
                std::ostringstream output;
                for (const auto& node : nodes)
                    output << node->path.name << " = " << node->to_string() << '\n';
                return output.str();
            }

            if (path.empty() || nodes.size() != 1)
                throw std::invalid_argument("Scalar output requires request to select exactly one value.");
            const auto& node = nodes.front();
            if (!node->value || !node->dimension.empty() || node->value->get_size() != 1 || node->units)
                throw std::invalid_argument("Scalar output requires a defined unitless, non-array value.");

            const std::string type = request.has_param("type") ? request.get_param_value("type") : "";
            const std::map<std::string, dip::NodeDtype> types = {
                {"bool", dip::NodeDtype::Boolean}, {"integer", dip::NodeDtype::Integer},
                {"float", dip::NodeDtype::Float}, {"string", dip::NodeDtype::String}
            };
            if (!type.empty()) {
                const auto expected = types.find(type);
                if (expected == types.end())
                    throw std::invalid_argument("The type query parameter must be bool, integer, float, or string.");
                if (node->dtype != expected->second)
                    throw std::invalid_argument("The selected value does not have the requested DIPL type: " + type);
            }
            core::StringFormatType format;
            format.stringQuotes = false;
            format.valuePrecision = std::numeric_limits<double>::max_digits10;
            return node->value->to_string(format) + '\n';
        }

    } // namespace

    void register_published_routes(httplib::Server& server, const std::vector<PublishedInput>& inputs) {
        const auto environments = std::make_shared<PublishedEnvironments>(inputs);
        server.Get("/snt/dip/environments", [environments](const httplib::Request&, httplib::Response& response) {
            response.set_content(environments->list(), "application/json");
        });
        server.Get("/snt/dip/environment", [environments](const httplib::Request& request, httplib::Response& response) {
            handle_json(response, [&] { return render_environment(environments->get(required_param(request, "name")), request); });
        });
    }

} // namespace snt::server
