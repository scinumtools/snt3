#include "server.h"

#include "snt/api/dip_parse.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <system_error>
#include <unordered_set>

namespace snt::server {

    namespace {

        std::filesystem::path safe_bundle_path(std::string filename) {
            std::replace(filename.begin(), filename.end(), '\\', '/');
            const std::filesystem::path path(filename);
            if (path.empty() || path.is_absolute() || path.has_root_directory() || path.has_root_name())
                throw std::invalid_argument("Bundle file names must be nonempty relative paths.");
            for (const auto& component : path) {
                if (component == "." || component == "..")
                    throw std::invalid_argument("Bundle file names must not contain . or .. path components.");
            }
            return path;
        }

        class ProjectBundle {
          private:
            std::filesystem::path directory;
            std::unordered_set<std::string> files;

            static std::filesystem::path create_directory() {
                std::random_device random;
                const auto temporary = std::filesystem::temp_directory_path();
                for (size_t attempt = 0; attempt < 32; ++attempt) {
                    const auto candidate = temporary / ("snt-server-" + std::to_string(random()) + "-" +
                                                        std::to_string(random()));
                    std::error_code error;
                    if (std::filesystem::create_directory(candidate, error))
                        return candidate;
                    if (error)
                        throw std::filesystem::filesystem_error("create_directory", candidate, error);
                }
                throw std::runtime_error("Unable to create a temporary directory for the DIP project bundle.");
            }

          public:
            ProjectBundle() : directory(create_directory()) {}

            ~ProjectBundle() {
                std::error_code error;
                std::filesystem::remove_all(directory, error);
            }

            ProjectBundle(const ProjectBundle&) = delete;
            ProjectBundle& operator=(const ProjectBundle&) = delete;

            void add_file(const std::filesystem::path& relative_path, const std::string& content) {
                const auto key = relative_path.generic_string();
                if (!files.insert(key).second)
                    throw std::invalid_argument("Each bundle file may be uploaded only once: " + key);

                const auto path = directory / relative_path;
                std::error_code error;
                std::filesystem::create_directories(path.parent_path(), error);
                if (error)
                    throw std::filesystem::filesystem_error("create_directories", path.parent_path(), error);

                std::ofstream output(path, std::ios::binary);
                if (!output)
                    throw std::runtime_error("Unable to write uploaded bundle file: " + key);
                output.write(content.data(), static_cast<std::streamsize>(content.size()));
                if (!output)
                    throw std::runtime_error("Unable to write uploaded bundle file: " + key);
            }

            std::filesystem::path project_file() const { return directory / "DIPfile"; }
            std::filesystem::path output_file(const std::string_view name) const { return directory / name; }
        };

        std::filesystem::path add_project_bundle(const httplib::Request& request, ProjectBundle& bundle) {
            if (!request.form.fields.empty())
                throw std::invalid_argument("A DIP project bundle accepts uploaded files only.");
            if (request.form.get_file_count("project") != 1)
                throw std::invalid_argument("A DIP project bundle requires exactly one uploaded project part.");

            const auto project = request.form.get_file("project");
            bundle.add_file("DIPfile", project.content);
            for (const auto& [name, file] : request.form.files) {
                if (name == "project")
                    continue;
                if (name != "file")
                    throw std::invalid_argument("Bundle uploads use one project part and file parts only.");
                bundle.add_file(safe_bundle_path(file.filename), file.content);
            }
            return bundle.project_file();
        }

        void add_dip_input(const httplib::Request& request, api::DIPParse& command, ProjectBundle& bundle) {
            if (!request.form.fields.empty() || !request.form.files.empty()) {
                command.argument_add("project", {add_project_bundle(request, bundle).string()});
                return;
            }
            if (request.body.empty())
                throw std::invalid_argument("The DIPL request body is empty.");
            command.argument_add("string", {request.body});
        }

        void configure_dip_query(const httplib::Request& request, api::DIPParse& command) {
            if (request.has_param("request"))
                command.argument_request(request.get_param_value("request"));
            if (request.has_param("tags"))
                command.argument_tags(split_tags(request.get_param_value("tags")));
        }

        std::string read_binary_file(const std::filesystem::path& path) {
            std::ifstream input(path, std::ios::binary);
            if (!input)
                throw std::runtime_error("Unable to read generated DIPH5 output.");
            return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        }

    } // namespace

    void register_dip_routes(httplib::Server& server) {
        server.Post("/snt/dip/parse", [](const httplib::Request& request, httplib::Response& response) {
            if (request.has_param("output")) {
                handle_response(response, [&] {
                    if (request.get_param_value("output") != "diph5")
                        throw std::invalid_argument("The output query parameter must be diph5.");
                    if (optional_flag(request, "value"))
                        throw std::invalid_argument("The value query parameter cannot be combined with output=diph5.");

                    ProjectBundle bundle;
                    api::DIPParse command;
                    add_dip_input(request, command, bundle);
                    configure_dip_query(request, command);
                    const auto output = bundle.output_file("environment.diph5");
                    command.argument_print();
                    command.argument_save(output.string());
                    command.execute();
                    response.set_header("Content-Disposition", "attachment; filename=environment.diph5");
                    response.set_content(read_binary_file(output), "application/x-hdf5");
                });
                return;
            }

            handle_json(response, [&] {
                ProjectBundle bundle;
                api::DIPParse command;
                add_dip_input(request, command, bundle);
                configure_dip_query(request, command);
                if (optional_flag(request, "value")) {
                    command.argument_value(request.has_param("type") ? request.get_param_value("type") : "");
                } else {
                    command.argument_print();
                }
                return command.execute();
            });
        });
    }

} // namespace snt::server
