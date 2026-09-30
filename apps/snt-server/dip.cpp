#include <map>
#include "server.h"

#include "snt/api/dip_parse.h"
#include <snt/api/dip_compare.h>
#include <snt/api/dip_report.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <unordered_set>
#include <vector>

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
            for (const auto& [name, field] : request.form.fields)
                if (name != "override")
                    throw std::invalid_argument("Project bundle fields must contain override text.");
            if (request.form.get_file_count("project") != 1)
                throw std::invalid_argument("A DIP project bundle requires exactly one uploaded project part.");

            const auto project = request.form.get_file("project");
            bundle.add_file("DIPfile", project.content);
            for (const auto& [name, file] : request.form.files) {
                if (name == "project" || name == "override")
                    continue;
                if (name != "file")
                    throw std::invalid_argument("Bundle uploads accept project, file, and override parts only.");
                bundle.add_file(safe_bundle_path(file.filename), file.content);
            }
            return bundle.project_file();
        }

        void add_dip_input(const httplib::Request& request, api::DIPParse& command, ProjectBundle& bundle) {
            if (!request.form.fields.empty() || !request.form.files.empty()) {
                if (request.form.files.count("project") || request.form.fields.count("project")) {
                    command.argument_add("project", {add_project_bundle(request, bundle).string()});
                    for (const auto& [name, part] : request.form.fields)
                        if (name == "override")
                            command.argument_add("override_string", {part.content});
                    for (const auto& [name, part] : request.form.files)
                        if (name == "override")
                            command.argument_add("override_string", {part.content});
                    return;
                }
                std::map<std::string, std::string> schemas;
                std::vector<std::string> overrides;
                std::string code;
                size_t code_count = 0;
                auto add_part = [&](const std::string& name, const std::string& content) {
                    if (name == "code") {
                        code = content;
                        ++code_count;
                    } else if (name == "override") {
                        overrides.push_back(content);
                    } else if (name.rfind("schema:", 0) == 0 && name.size() > 7) {
                        if (!schemas.emplace(name.substr(7), content).second)
                            throw std::invalid_argument("Duplicate schema part: " + name);
                    } else {
                        throw std::invalid_argument("DIPL uploads accept code, override, and schema:<name> parts only.");
                    }
                };
                for (const auto& [name, part] : request.form.fields)
                    add_part(name, part.content);
                for (const auto& [name, part] : request.form.files)
                    add_part(name, part.content);
                if (code_count != 1)
                    throw std::invalid_argument("Schema uploads require exactly one code part.");
                for (const auto& [name, content] : schemas)
                    command.argument_add("schema_string", {name, content});
                for (const auto& content : overrides)
                    command.argument_add("override_string", {content});
                command.argument_add("string", {code});
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
                throw std::runtime_error("Unable to read generated output.");
            return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        }

        dip::report::ReportFormat report_format(const httplib::Request& request) {
            const auto format = request.has_param("format") ? request.get_param_value("format") : "tex";
            if (format == "tex") return dip::report::ReportFormat::Tex;
            if (format == "pdf") return dip::report::ReportFormat::Pdf;
            if (format == "md") return dip::report::ReportFormat::Markdown;
            if (format == "rst") return dip::report::ReportFormat::Rst;
            if (format == "html") return dip::report::ReportFormat::Html;
            if (format == "typ") return dip::report::ReportFormat::Typst;
            if (format == "txt") return dip::report::ReportFormat::Text;
            if (format == "json") return dip::report::ReportFormat::Json;
            throw std::invalid_argument("The format query parameter must be tex, pdf, md, rst, html, typ, txt, or json.");
        }

        std::pair<const char*, const char*> report_response_type(dip::report::ReportFormat format) {
            using Format = dip::report::ReportFormat;
            switch (format) {
            case Format::Tex: return {"report.tex", "application/x-tex"};
            case Format::Pdf: return {"report.pdf", "application/pdf"};
            case Format::Markdown: return {"report.md", "text/markdown"};
            case Format::Rst: return {"report.rst", "text/x-rst"};
            case Format::Html: return {"report.html", "text/html"};
            case Format::Typst: return {"report.typ", "text/plain"};
            case Format::Text: return {"report.txt", "text/plain"};
            case Format::Json: return {"report.json", "application/json"};
            }
            throw std::invalid_argument("Unknown report format.");
        }

        dip::report::ReportOptions report_options(const httplib::Request& request) {
            dip::report::ReportOptions options;
            options.input_label = request.has_param("input_label") ? request.get_param_value("input_label") : "DIPL request";
            if (request.has_param("title")) options.title = request.get_param_value("title");
            if (request.has_param("author")) options.author = request.get_param_value("author");
            if (request.has_param("date")) options.date = request.get_param_value("date");
            if (request.has_param("version")) options.version = request.get_param_value("version");
            return options;
        }

        std::size_t compare_count(const httplib::Request& request, const char* name, std::size_t fallback) {
            if (!request.has_param(name)) return fallback;
            const auto value = request.get_param_value(name);
            if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
                throw std::invalid_argument(std::string(name) + " must be a nonnegative integer.");
            const auto count = std::stoull(value);
            if (count > std::numeric_limits<std::size_t>::max())
                throw std::invalid_argument(std::string(name) + " is too large.");
            return static_cast<std::size_t>(count);
        }

        std::string comparison_json(const dip::ComparisonResult& result, std::size_t max_details) {
            std::ostringstream out;
            out << "{\"scope\":\"" << (result.scope == dip::ComparisonScope::Full ? "full" : "effective")
                << "\",\"equal\":" << (result.equal() ? "true" : "false")
                << ",\"added\":" << result.added << ",\"removed\":" << result.removed
                << ",\"changed\":" << result.changed << ",\"differences\":[";
            const auto count = std::min(max_details, result.differences.size());
            for (std::size_t i = 0; i < count; ++i) {
                const auto& difference = result.differences[i];
                if (i) out << ',';
                out << "{\"path\":\"" << json_escape(difference.path)
                    << "\",\"category\":\"" << json_escape(difference.category)
                    << "\",\"kind\":\"" << (difference.kind == dip::DifferenceKind::Added ? "added" :
                                              difference.kind == dip::DifferenceKind::Removed ? "removed" : "changed")
                    << "\",\"fields\":[";
                for (std::size_t j = 0; j < difference.fields.size(); ++j) {
                    if (j) out << ',';
                    out << '"' << json_escape(difference.fields[j]) << '"';
                }
                out << "],\"before\":\"" << json_escape(difference.before)
                    << "\",\"after\":\"" << json_escape(difference.after)
                    << "\",\"changed_elements\":" << difference.changed_elements
                    << ",\"example_indices\":[";
                for (std::size_t j = 0; j < difference.example_indices.size(); ++j) {
                    if (j) out << ',';
                    out << difference.example_indices[j];
                }
                out << "]}";
            }
            out << "],\"omitted\":" << result.differences.size() - count << "}\n";
            return out.str();
        }

    } // namespace

    void register_dip_routes(httplib::Server& server) {
        server.Post("/snt/dip/compare", [](const httplib::Request& request, httplib::Response& response) {
            handle_response(response, [&] {
                if (!request.form.fields.empty() || request.form.get_file_count("before") != 1 ||
                    request.form.get_file_count("after") != 1 || request.form.files.size() != 2)
                    throw std::invalid_argument("Comparison requires exactly one before and one after DIPH5 upload.");
                const auto format = request.has_param("format") ? request.get_param_value("format") : "json";
                if (format != "json" && format != "text")
                    throw std::invalid_argument("Comparison format must be json or text.");
                dip::ComparisonOptions options;
                const auto scope = request.has_param("scope") ? request.get_param_value("scope") : "effective";
                if (scope == "full") options.scope = dip::ComparisonScope::Full;
                else if (scope != "effective")
                    throw std::invalid_argument("Comparison scope must be effective or full.");
                options.max_array_examples = compare_count(request, "max_array_examples", 3);
                const auto max_details = compare_count(request, "max_details", 50);
                ProjectBundle bundle;
                bundle.add_file("before.diph5", request.form.get_file("before").content);
                bundle.add_file("after.diph5", request.form.get_file("after").content);
                api::DIPCompare command(bundle.output_file("before.diph5"), bundle.output_file("after.diph5"));
                command.set_options(options);
                const auto result = command.compare();
                if (format == "text") response.set_content(api::render_dip_comparison(result, max_details), "text/plain");
                else response.set_content(comparison_json(result, max_details), "application/json");
            });
        });
        server.Post("/snt/dip/report", [](const httplib::Request& request, httplib::Response& response) {
            handle_response(response, [&] {
                const auto format = report_format(request);
                ProjectBundle bundle;
                api::DIPParse command;
                add_dip_input(request, command, bundle);
                const auto [filename, content_type] = report_response_type(format);
                const auto output = bundle.output_file(filename);
                auto options = report_options(request);
                options.source_root = bundle.project_file().parent_path();
                api::generate_dip_report(command.evaluate(), format, output, options);
                response.set_header("Content-Disposition", "attachment; filename=" + std::string(filename));
                response.set_content(read_binary_file(output), content_type);
            });
        });
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
