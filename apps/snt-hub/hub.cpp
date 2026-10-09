#include "hub.h"
#include "process.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace snt::hub {
namespace {
namespace fs = std::filesystem;
using Json = nlohmann::json;
constexpr const char* catalog_url = "https://scinumtools.github.io/snt-hub/catalog/v1.json";
constexpr std::size_t catalog_limit = 2'000'000;

std::string environment(const char* name) {
    const char* value = std::getenv(name);
    return value ? value : "";
}

bool local_fixture() { return environment("SNT_HUB_LOCAL_FIXTURE") == "1"; }

std::string text(const Json& object, const char* key) {
    if (!object.is_object() || !object.contains(key) || !object[key].is_string())
        throw Error(std::string("Missing or invalid Hub field: ") + key);
    return object[key].get<std::string>();
}

int number(const Json& object, const char* key) {
    if (!object.is_object() || !object.contains(key) || !object[key].is_number_integer())
        throw Error(std::string("Missing or invalid Hub field: ") + key);
    return object[key].get<int>();
}

std::string sha(const std::string& value, const char* label) {
    static const std::regex pattern("[0-9a-f]{40}");
    if (!std::regex_match(value, pattern))
        throw Error(std::string("Invalid ") + label + ": expected a full 40-character Git SHA");
    return value;
}

std::string identifier(const std::string& value, const char* label) {
    static const std::regex pattern("[a-z0-9][a-z0-9_-]*");
    if (!std::regex_match(value, pattern)) throw Error(std::string("Invalid ") + label);
    return value;
}

fs::path relative_path(const std::string& value, const char* label) {
    if (value.empty() || value.find('\\') != std::string::npos || value.front() == '/' ||
        value.find(':') != std::string::npos)
        throw Error(std::string("Invalid ") + label + ": expected a relative Hub path");
    fs::path path(value);
    if (path.is_absolute()) throw Error(std::string("Invalid ") + label + ": expected a relative Hub path");
    for (const auto& part : path)
        if (part == "." || part == ".." || part.empty())
            throw Error(std::string("Invalid ") + label + ": path must stay inside the Hub checkout");
    return path;
}

fs::path inside(const fs::path& root, const fs::path& relative) {
    const auto base = fs::weakly_canonical(root);
    const auto resolved = fs::weakly_canonical(base / relative);
    const auto difference = resolved.lexically_relative(base);
    if (difference.empty() || *difference.begin() == "..")
        throw Error("Hub path escapes its checkout: " + relative.string());
    return resolved;
}

void validate_project(const Json& project) {
    const auto id = identifier(text(project, "id"), "project ID");
    sha(text(project, "source_revision"), "source revision");
    const auto source_url = text(project, "source_url");
    if (source_url.rfind("https://", 0) != 0 && !(local_fixture() && source_url.rfind("file://", 0) == 0))
        throw Error("Invalid source URL for " + id);
    if (!project.contains("hub") || !project["hub"].is_object())
        throw Error("Missing Hub adapter metadata for " + id);
    const auto& hub = project["hub"];
    if (number(hub, "adapter_protocol") != 1)
        throw Error("Unsupported adapter protocol for " + id);
    for (const auto* key : {"runtime_package", "adapter_package", "setup_manifest"})
        relative_path(text(hub, key), key);
    static const std::regex executable_pattern("[A-Za-z0-9][A-Za-z0-9_.-]*");
    if (!std::regex_match(text(hub, "adapter_executable"), executable_pattern))
        throw Error("Invalid adapter executable for " + id);
    if (!project.contains("setups") || !project["setups"].is_object())
        throw Error("Missing setup catalogue for " + id);
}

Json validate_catalog(const Json& data) {
    if (!data.is_object() || number(data, "schema_version") != 1)
        throw Error("Unsupported Hub catalogue schema");
    sha(text(data, "hub_revision"), "Hub revision");
    const auto repository = text(data, "hub_repository");
    if (repository.rfind("https://", 0) != 0 && !(local_fixture() && repository.rfind("file://", 0) == 0))
        throw Error("Invalid Hub repository URL");
    if (!data.contains("projects") || !data["projects"].is_array())
        throw Error("Hub catalogue has no projects array");
    std::vector<std::string> ids;
    for (const auto& project : data["projects"]) {
        validate_project(project);
        const auto id = text(project, "id");
        if (std::find(ids.begin(), ids.end(), id) != ids.end()) throw Error("Duplicate Hub project: " + id);
        ids.push_back(id);
    }
    return data;
}

Json read_json(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw Error("Cannot read Hub file: " + path.string(), 5);
    try { return Json::parse(stream); }
    catch (const Json::exception& error) { throw Error("Invalid Hub JSON in " + path.string() + ": " + error.what(), 5); }
}

void write_json(const fs::path& path, const Json& data) {
    fs::create_directories(path.parent_path());
    const auto temporary = path.string() + ".tmp";
    {
        std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
        if (!stream) throw Error("Cannot write Hub file: " + temporary, 5);
        stream << data.dump(2) << '\n';
        if (!stream) throw Error("Cannot write Hub file: " + temporary, 5);
    }
#ifdef _WIN32
    std::error_code ignored;
    fs::remove(path, ignored);
#endif
    fs::rename(temporary, path);
}

std::size_t curl_write(char* data, std::size_t width, std::size_t count, void* context) {
    const auto size = width * count;
    auto& output = *static_cast<std::string*>(context);
    if (output.size() + size > catalog_limit) return 0;
    output.append(data, size);
    return size;
}

Json catalog(const fs::path& root, bool allow_cache = true) {
    const auto configured = environment("SNT_HUB_CATALOG_URL");
    const auto url = configured.empty() ? std::string(catalog_url) : configured;
    if (url.rfind("https://", 0) != 0 && !local_fixture())
        throw Error("The Hub catalogue must use HTTPS");
    const auto cache = root / "catalogue-v1.json";
    std::string body;
    CURL* curl = curl_easy_init();
    if (!curl) throw Error("Cannot initialize HTTPS for the Hub catalogue", 3);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, local_fixture() ? CURLPROTO_HTTPS | CURLPROTO_FILE : CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    const auto status = curl_easy_perform(curl);
    long http_status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
    curl_easy_cleanup(curl);
    if (status == CURLE_OK && (http_status == 0 || (http_status >= 200 && http_status < 300))) {
        try {
            const auto data = validate_catalog(Json::parse(body));
            write_json(cache, data);
            return data;
        } catch (const Json::exception& error) {
            throw Error(std::string("Invalid Hub catalogue JSON: ") + error.what(), 3);
        }
    }
    if (allow_cache && fs::is_regular_file(cache)) return validate_catalog(read_json(cache));
    throw Error("Cannot load Hub catalogue (HTTPS or file error " + std::to_string(status) +
                ", status " + std::to_string(http_status) + ")", 3);
}

Json project_from(const Json& data, const std::string& id) {
    for (const auto& project : data.at("projects"))
        if (text(project, "id") == id) return project;
    throw Error("Unknown Hub project: " + id);
}

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string run(const std::vector<std::string>& args, bool capture = true) {
    const auto result = process(args, capture);
    if (result.status == 127) throw Error("Missing prerequisite: " + args.front(), 4);
    if (result.status != 0)
        throw Error("Command failed (" + std::to_string(result.status) + "): " + args.front() +
                    (result.output.empty() ? "" : "\n" + trim(result.output)), 5);
    return trim(result.output);
}

std::string git(const std::vector<std::string>& args) {
    std::vector<std::string> command{"git"};
    command.insert(command.end(), args.begin(), args.end());
    return run(command);
}

std::string git_at(const fs::path& checkout, const std::vector<std::string>& args) {
    std::vector<std::string> command{"-C", checkout.string()};
    command.insert(command.end(), args.begin(), args.end());
    return git(command);
}

fs::path data_root(const std::string& prefix) {
    if (!prefix.empty()) return fs::absolute(fs::path(prefix)).lexically_normal();
#ifdef _WIN32
    const auto local = environment("LOCALAPPDATA");
    if (!local.empty()) return fs::path(local) / "SNT/hub";
#elif defined(__APPLE__)
    return fs::path(environment("HOME")) / "Library/Application Support/snt/hub";
#else
    const auto xdg = environment("XDG_DATA_HOME");
    if (!xdg.empty()) return fs::path(xdg) / "snt/hub";
#endif
    return fs::path(environment("HOME")) / ".local/share/snt/hub";
}

fs::path runtime_python(const fs::path& runtime) {
#ifdef _WIN32
    return runtime / "Scripts/python.exe";
#else
    return runtime / "bin/python";
#endif
}

fs::path adapter_path(const fs::path& runtime, const std::string& name) {
#ifdef _WIN32
    return runtime / "Scripts" / (name + ".exe");
#else
    return runtime / "bin" / name;
#endif
}

Json package_versions(const fs::path& runtime) {
    const auto output = run({runtime_python(runtime).string(), "-m", "pip", "list", "--format=json"});
    std::istringstream lines(output);
    for (std::string line; std::getline(lines, line); ) {
        line = trim(line);
        if (line.rfind("[{", 0) != 0 || line.back() != ']') continue;
        try { return Json::parse(line); }
        catch (const Json::exception& error) {
            throw Error(std::string("Cannot parse installed adapter package versions: ") + error.what(), 5);
        }
    }
    throw Error("Cannot read installed adapter package versions", 5);
}

void checkout_matches(const fs::path& checkout, const Json& lock) {
    const auto hub_revision = sha(text(lock, "hub_revision"), "Hub revision");
    const auto source_revision = sha(text(lock, "source_revision"), "source revision");
    const auto source_path = relative_path(text(lock, "source_path"), "source path");
    if (git_at(checkout, {"rev-parse", "HEAD"}) != hub_revision)
        throw Error("Installed Hub checkout does not match its lock", 5);
    const auto tree = git_at(checkout, {"ls-tree", "HEAD", "--", source_path.generic_string()});
    if (tree.rfind("160000 commit " + source_revision + "\t", 0) != 0)
        throw Error("Hub gitlink does not match its source revision", 5);
    const auto source = inside(checkout, source_path);
    if (git_at(source, {"rev-parse", "HEAD"}) != source_revision)
        throw Error("Installed source checkout does not match its lock", 5);
    if (!git_at(checkout, {"status", "--porcelain", "--untracked-files=no", "--ignore-submodules=dirty"}).empty())
        throw Error("Installed Hub checkout has modified tracked files", 5);
    if (!git_at(source, {"status", "--porcelain", "--untracked-files=no"}).empty())
        throw Error("Installed source checkout has modified tracked files", 5);
}

struct Installation { fs::path directory; Json lock; };

std::optional<Installation> active(const fs::path& root, const std::string& id) {
    const auto selection = root / "active" / (id + ".json");
    if (!fs::is_regular_file(selection)) return std::nullopt;
    const auto revision = sha(text(read_json(selection), "hub_revision"), "active Hub revision");
    const auto directory = root / "installations" / id / revision;
    const auto lock = read_json(directory / "install-lock.json");
    if (number(lock, "schema_version") != 1 || number(lock, "adapter_protocol") != 1 ||
        text(lock, "project") != id || text(lock, "hub_revision") != revision)
        throw Error("Installed Hub lock disagrees with the active selection", 5);
    static const std::regex executable_pattern("[A-Za-z0-9][A-Za-z0-9_.-]*");
    if (!std::regex_match(text(lock, "adapter_executable"), executable_pattern))
        throw Error("Installed adapter executable is invalid", 5);
    return Installation{directory, lock};
}

void select_active(const fs::path& root, const std::string& id, const std::string& revision) {
    write_json(root / "active" / (id + ".json"), Json{{"hub_revision", revision}});
}

struct TemporaryDirectory {
    fs::path path;
    explicit TemporaryDirectory(const fs::path& parent, const std::string& prefix) {
        std::random_device random;
        for (int attempt = 0; attempt < 100; ++attempt) {
            const auto candidate = parent / ("." + prefix + "-" + std::to_string(random()) + "-" +
                                             std::to_string(attempt));
            if (fs::create_directory(candidate)) { path = candidate; return; }
        }
        throw Error("Cannot create Hub installation staging directory", 5);
    }
    ~TemporaryDirectory() {
        if (!path.empty()) {
            std::error_code ignored;
            fs::remove_all(path, ignored);
        }
    }
};

std::pair<Json, fs::path> verified_project(const fs::path& checkout, const Json& advertised) {
    const auto id = text(advertised, "id");
    const auto path = inside(checkout, fs::path("projects") / id / "project.json");
    const auto record = read_json(path);
    for (const auto* key : {"id", "source_url", "source_revision"})
        if (text(record, key) != text(advertised, key))
            throw Error(std::string("Catalogue and pinned project disagree on ") + key, 5);
    const auto& hub = record.at("hub");
    const auto& listed_hub = advertised.at("hub");
    for (const auto* key : {"runtime_package", "adapter_package", "adapter_executable", "setup_manifest"})
        if (text(hub, key) != text(listed_hub, key))
            throw Error(std::string("Catalogue and pinned project disagree on hub.") + key, 5);
    if (number(hub, "adapter_protocol") != number(listed_hub, "adapter_protocol"))
        throw Error("Catalogue and pinned project disagree on adapter protocol", 5);
    const auto source = fs::path("projects") / id / "source";
    if (relative_path(text(record, "source_path"), "source path") != source)
        throw Error("Project source path does not match its Git submodule", 5);
    const auto canonical_url = [](std::string url) {
        if (url.size() >= 4 && url.substr(url.size() - 4) == ".git") url.resize(url.size() - 4);
        return url;
    };
    const auto module_url = git_at(checkout, {"config", "-f", ".gitmodules", "--get",
        "submodule." + source.generic_string() + ".url"});
    if (canonical_url(module_url) != canonical_url(text(record, "source_url")))
        throw Error("Pinned Git submodule URL disagrees with the project source URL", 5);
    for (const auto* key : {"runtime_package", "adapter_package", "setup_manifest"})
        if (!fs::exists(inside(checkout, relative_path(text(hub, key), key))))
            throw Error(std::string("Pinned Hub package or manifest is missing: ") + key, 5);
    return {record, source};
}

void install(const fs::path& root, const std::string& id, const std::string& requested, bool update) {
    const auto data = catalog(root, false);
    const auto project = project_from(data, id);
    const auto selected = sha(requested.empty() ? text(data, "hub_revision") : requested,
                              "requested Hub revision");
    if (selected != text(data, "hub_revision"))
        throw Error("Requested revision is not in the published catalogue; use a published revision");
    const auto previous = active(root, id);
    if (previous && text(previous->lock, "hub_revision") != selected && !update)
        throw Error("A different Hub revision is installed; pass --update to switch explicitly");
    const auto final = root / "installations" / id / selected;
    if (fs::exists(final)) {
        if (!fs::is_regular_file(final / "install-lock.json"))
            throw Error("Incomplete installation at " + final.string() + "; remove it before retrying", 5);
        const auto lock = read_json(final / "install-lock.json");
        if (text(lock, "hub_revision") != selected || text(lock, "source_revision") != text(project, "source_revision"))
            throw Error("Installed lock does not match the published catalogue", 5);
        checkout_matches(final / "hub", lock);
        if (!fs::is_regular_file(adapter_path(final / "runtime", text(lock, "adapter_executable"))))
            throw Error("Installed adapter executable is missing", 5);
        select_active(root, id, selected);
        std::cout << "Already installed " << id << " at " << selected << ": " << final << '\n';
        return;
    }

    fs::create_directories(final.parent_path());
    TemporaryDirectory temporary(final.parent_path(), id + "-install");
    const auto stage = temporary.path / "bundle";
    fs::create_directory(stage);
    auto checkout = stage / "hub";
    git({"clone", "--no-checkout", text(data, "hub_repository"), checkout.string()});
    git_at(checkout, {"checkout", "--detach", selected});
    const auto [record, source] = verified_project(checkout, project);
    git_at(checkout, {"submodule", "update", "--init", "--", source.generic_string()});
    Json lock{
        {"schema_version", 1}, {"project", id},
        {"hub_repository", text(data, "hub_repository")}, {"hub_revision", selected},
        {"source_url", text(record, "source_url")},
        {"source_revision", text(record, "source_revision")},
        {"source_path", source.generic_string()},
        {"adapter_executable", text(record.at("hub"), "adapter_executable")},
        {"adapter_protocol", 1},
        {"runtime_path", "runtime"},
        {"runtime_package", text(record.at("hub"), "runtime_package")},
        {"adapter_package", text(record.at("hub"), "adapter_package")},
        {"setup_manifest", text(record.at("hub"), "setup_manifest")},
    };
    checkout_matches(checkout, lock);
    fs::rename(stage, final);
    try {
        checkout = final / "hub";
        const auto runtime = final / "runtime";
#ifdef _WIN32
        const std::string default_python = "python";
#else
        const std::string default_python = "python3";
#endif
        const auto python = environment("SNT_HUB_PYTHON").empty()
            ? default_python : environment("SNT_HUB_PYTHON");
        run({python, "-m", "venv", runtime.string()});
        const auto managed_python = runtime_python(runtime).string();
        run({managed_python, "-m", "pip", "install", "--disable-pip-version-check", "--no-input",
             inside(checkout, relative_path(text(lock, "runtime_package"), "runtime package")).string(),
             inside(checkout, relative_path(text(lock, "adapter_package"), "adapter package")).string()});
        if (!fs::is_regular_file(adapter_path(runtime, text(lock, "adapter_executable"))))
            throw Error("Adapter package did not install its declared executable", 5);
        lock["packages"] = Json::array();
        std::istringstream packages(run({managed_python, "-m", "pip", "freeze", "--all"}));
        for (std::string line; std::getline(packages, line); )
            if (!line.empty()) lock["packages"].push_back(line);
        lock["package_versions"] = package_versions(runtime);
        write_json(final / "install-lock.json", lock);
    } catch (...) {
        std::error_code ignored;
        fs::remove_all(final, ignored);
        throw;
    }
    select_active(root, id, selected);
    std::cout << "Installed " << id << " at Hub " << selected << ", source "
              << text(lock, "source_revision") << ": " << final << '\n';
}

Json installed_setups(const Installation& installed) {
    const auto checkout = installed.directory / "hub";
    checkout_matches(checkout, installed.lock);
    const auto manifest = inside(checkout, relative_path(text(installed.lock, "setup_manifest"), "setup manifest"));
    const auto data = read_json(manifest);
    if (number(data, "schema_version") != 1 || !data.contains("setups") || !data["setups"].is_object())
        throw Error("Unsupported installed setup manifest", 5);
    return data["setups"];
}

void examples(const fs::path& root, const std::string& id) {
    const auto installed = active(root, id);
    const auto setups = installed ? installed_setups(*installed) : project_from(catalog(root), id).at("setups");
    for (auto item = setups.begin(); item != setups.end(); ++item) {
        const auto capability = text(item.value(), "capability");
        if (capability != "complete" && capability != "native-inputs-only" && capability != "requires-external-data")
            throw Error("Invalid capability for setup " + item.key());
        std::cout << item.key() << '\t' << capability << '\n';
    }
}

int setup(const fs::path& root, const std::string& id, const std::string& name,
          const std::string& output, const std::string& override_file, bool inputs_only) {
    const auto installed = active(root, id);
    if (!installed) throw Error(id + " is not installed; run snt hub install " + id);
    const auto setups = installed_setups(*installed);
    if (!setups.contains(name)) throw Error("Unknown setup for " + id + ": " + name);
    const auto capability = text(setups.at(name), "capability");
    if (capability != "complete" && !inputs_only)
        throw Error(name + " is " + capability + "; use --inputs-only");
    if (output.empty()) throw Error("Specify --output DIR for hub setup");
    const auto destination = fs::absolute(fs::path(output)).lexically_normal();
    if (fs::symlink_status(destination).type() != fs::file_type::not_found)
        throw Error("Output already exists: " + destination.string());
    const auto& lock = installed->lock;
    const auto checkout = installed->directory / "hub";
    const auto adapter = adapter_path(installed->directory / "runtime", text(lock, "adapter_executable"));
    if (!fs::is_regular_file(adapter)) throw Error("Installed adapter executable is missing", 5);
    if (lock.contains("package_versions") &&
        package_versions(installed->directory / "runtime") != lock.at("package_versions"))
        throw Error("Installed adapter runtime package versions no longer match its lock", 5);
    std::vector<std::string> command{adapter.string(), "setup", "--bundle",
        inside(checkout, relative_path(text(lock, "adapter_package"), "adapter package")).string(),
        "--source", inside(checkout, relative_path(text(lock, "source_path"), "source path")).string(),
        "--setup", name, "--output", destination.string()};
    if (inputs_only) command.push_back("--inputs-only");
    if (!override_file.empty()) {
        const auto candidate = fs::absolute(fs::path(override_file));
        if (!fs::is_regular_file(candidate)) throw Error("Override file does not exist: " + candidate.string());
        command.push_back("--override-file");
        command.push_back(fs::canonical(candidate).string());
    }
    const auto result = process(command, false);
    if (result.status == 127) throw Error("Installed adapter executable is unavailable", 4);
    if (result.status == 0)
        std::cout << "Hub revision: " << text(lock, "hub_revision") << '\n'
                  << "Source revision: " << text(lock, "source_revision") << '\n'
                  << "Setup lock: " << (destination / "setup-lock.json") << '\n';
    return result.status;
}

void help(const std::string& action = {}) {
    if (action == "setup") {
        std::cout << "Usage: snt hub setup PROJECT EXAMPLE --output DIR [--override-file PATH] [--inputs-only] [--prefix DIR]\n"
                     "The override file contains bare DIPL assignments. Complete recipes accept only IC-safe targets.\n";
    } else if (action == "install") {
        std::cout << "Usage: snt hub install PROJECT [--revision SHA] [--update] [--prefix DIR]\n";
    } else {
        std::cout << "Usage: snt hub {list|install|examples|info|setup} [arguments] [--prefix DIR]\n"
                     "Install and prepare pinned SNT Hub code examples.\n"
                     "  list                   Show projects in the published catalogue\n"
                     "  install PROJECT        Install a pinned Hub bundle and adapter\n"
                     "  examples PROJECT       Show recipe capabilities\n"
                     "  info PROJECT           Show the installation lock\n"
                     "  setup PROJECT EXAMPLE  Prepare an example; see 'snt hub setup --help'\n";
    }
}

struct Arguments {
    std::string action, project, example, prefix, output, override_file, revision;
    bool update = false, inputs_only = false, show_help = false;
};

Arguments parse(int argc, char* argv[]) {
    Arguments args;
    std::vector<std::string> positionals;
    for (int i = 2; i < argc; ++i) {
        const std::string value(argv[i]);
        if (value == "-h" || value == "--help") args.show_help = true;
        else if (value == "--update") args.update = true;
        else if (value == "--inputs-only") args.inputs_only = true;
        else if (value == "--prefix" || value == "--output" || value == "--override-file" || value == "--revision") {
            if (++i >= argc) throw Error(value + " requires one value");
            const std::string selected(argv[i]);
            auto* destination = value == "--prefix" ? &args.prefix : value == "--output" ? &args.output :
                value == "--override-file" ? &args.override_file : &args.revision;
            if (!destination->empty()) throw Error(value + " may be specified only once");
            *destination = selected;
        } else if (!value.empty() && value.front() == '-') throw Error("Unknown hub option: " + value);
        else positionals.push_back(value);
    }
    if (!positionals.empty()) args.action = positionals.front();
    if (positionals.size() > 1) args.project = positionals[1];
    if (positionals.size() > 2) args.example = positionals[2];
    if (positionals.size() > 3) throw Error("Too many hub command arguments");
    if (args.show_help) return args;
    if (args.action != "list" && args.action != "install" && args.action != "examples" &&
        args.action != "info" && args.action != "setup")
        throw Error("Unknown hub command. Use 'snt hub --help'.");
    if (args.action != "list" && args.project.empty()) throw Error("Specify a Hub project");
    if (args.action == "setup" && args.example.empty()) throw Error("Specify a Hub example");
    if ((args.action == "list" && positionals.size() != 1) ||
        (args.action == "setup" && positionals.size() != 3) ||
        (args.action != "list" && args.action != "setup" && positionals.size() != 2))
        throw Error("Unexpected hub command arguments");
    if (args.action == "setup" && args.output.empty()) throw Error("Specify --output DIR for hub setup");
    if (args.action != "setup" && (!args.output.empty() || !args.override_file.empty() || args.inputs_only))
        throw Error("Setup options require 'snt hub setup'");
    if (args.action != "install" && (!args.revision.empty() || args.update))
        throw Error("Install options require 'snt hub install'");
    return args;
}

} // namespace

int command(int argc, char* argv[]) {
    const auto args = parse(argc, argv);
    if (args.show_help) { help(args.action); return 0; }
    const auto root = data_root(args.prefix);
    if (args.action == "list") {
        for (const auto& project : catalog(root).at("projects"))
            std::cout << text(project, "id") << '\t' << text(project, "name") << '\t'
                      << text(project, "source_revision") << '\n';
    } else if (args.action == "install") {
        install(root, args.project, args.revision, args.update);
    } else if (args.action == "examples") {
        examples(root, args.project);
    } else if (args.action == "info") {
        const auto installed = active(root, args.project);
        if (!installed) throw Error(args.project + " is not installed");
        checkout_matches(installed->directory / "hub", installed->lock);
        std::cout << installed->lock.dump(2) << '\n';
    } else if (args.action == "setup") {
        return setup(root, args.project, args.example, args.output, args.override_file, args.inputs_only);
    }
    return 0;
}

} // namespace snt::hub
