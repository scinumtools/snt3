#include "hub.h"
#include "process.h"
#include <snt/sha256.h>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
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
    if (hub.contains("build")) {
        const auto& build = hub.at("build");
        if (!build.is_object() || number(build, "protocol") != 1)
            throw Error("Unsupported build protocol for " + id);
        if (!build.contains("profiles") || !build.at("profiles").is_array() || build.at("profiles").empty())
            throw Error("Build profiles must be a nonempty array for " + id);
        std::vector<std::string> profiles;
        for (const auto& entry : build.at("profiles")) {
            if (!entry.is_string()) throw Error("Invalid build profile for " + id);
            const auto name = identifier(entry.get<std::string>(), "build profile");
            if (std::find(profiles.begin(), profiles.end(), name) != profiles.end())
                throw Error("Duplicate build profile for " + id);
            profiles.push_back(name);
        }
        if (std::find(profiles.begin(), profiles.end(), text(build, "default_profile")) == profiles.end())
            throw Error("Default build profile is not declared for " + id);
        if (!build.contains("requires_setup") || !build.at("requires_setup").is_boolean())
            throw Error("Missing or invalid build requires_setup for " + id);
    }
    if (hub.contains("run")) {
        const auto& execution = hub.at("run");
        if (!execution.is_object() || number(execution, "protocol") != 1)
            throw Error("Unsupported run protocol for " + id);
        if (!execution.contains("modes") || !execution.at("modes").is_array() || execution.at("modes").empty())
            throw Error("Run modes must be a nonempty array for " + id);
        for (const auto& mode : execution.at("modes"))
            if (!mode.is_string() || identifier(mode.get<std::string>(), "run mode").empty())
                throw Error("Invalid run mode for " + id);
    }
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

Json catalog() {
    const auto configured = environment("SNT_HUB_CATALOG_URL");
    const auto url = configured.empty() ? std::string(catalog_url) : configured;
    if (url.rfind("https://", 0) != 0 && !local_fixture())
        throw Error("The Hub catalogue must use HTTPS");
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
            return validate_catalog(Json::parse(body));
        } catch (const Json::exception& error) {
            throw Error(std::string("Invalid Hub catalogue JSON: ") + error.what(), 3);
        }
    }
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

std::string file_sha256(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw Error("Cannot read file for SHA-256: " + path.string(), 5);
    snt::Sha256 hash;
    std::array<char, 64 * 1024> buffer{};
    while (stream) {
        stream.read(buffer.data(), buffer.size());
        hash.update(buffer.data(), static_cast<std::size_t>(stream.gcount()));
    }
    if (stream.bad()) throw Error("Cannot finish reading file for SHA-256: " + path.string(), 5);
    return hash.finish();
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

struct TemporaryDirectory {
    fs::path path;
    explicit TemporaryDirectory(const fs::path& parent, const std::string& prefix) {
        std::random_device random;
        for (int attempt = 0; attempt < 100; ++attempt) {
            const auto candidate = parent / ("." + prefix + "-" + std::to_string(random()) + "-" +
                                             std::to_string(attempt));
            if (fs::create_directory(candidate)) { path = candidate; return; }
        }
        throw Error("Cannot create Hub staging directory", 5);
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
    for (const auto* key : {"build", "run"})
        if (hub.value(key, Json()) != listed_hub.value(key, Json()))
            throw Error(std::string("Catalogue and pinned project disagree on hub.") + key, 5);
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

struct Workspace { fs::path root; Json lock; };

fs::path workspace_root(const std::string& selected) {
    if (!selected.empty()) {
        const auto root = fs::absolute(selected).lexically_normal();
        if (!fs::is_regular_file(root / ".snthub/lock.json"))
            throw Error("No SNT Hub workspace at " + root.string());
        return root;
    }
    auto path = fs::current_path();
    while (true) {
        if (fs::is_regular_file(path / ".snthub/lock.json")) return path;
        if (path == path.root_path()) break;
        path = path.parent_path();
    }
    throw Error("No SNT Hub workspace found; run 'snt hub fetch PROJECT' first or use --workspace DIR");
}

Workspace workspace(const std::string& selected) {
    const auto root = workspace_root(selected);
    const auto lock = read_json(root / ".snthub/lock.json");
    if (number(lock, "schema_version") != 1 || number(lock, "adapter_protocol") != 1 ||
        number(lock, "materialization_version") != 1)
        throw Error("Unsupported SNT Hub workspace lock", 5);
    identifier(text(lock, "project"), "project ID");
    sha(text(lock, "hub_revision"), "Hub revision");
    sha(text(lock, "source_revision"), "source revision");
    return {root, lock};
}

void verify_workspace(const Workspace& current) {
    const auto& root = current.root;
    const auto& lock = current.lock;
    for (const auto& entry : {".snthub", "source", "dipl"})
        if (fs::is_symlink(root / entry) || !fs::is_directory(root / entry))
            throw Error("Workspace path is missing or is a symlink: " + (root / entry).string(), 5);
    const auto hub = root / ".snthub/hub";
    if (fs::is_symlink(hub) || !fs::is_directory(hub))
        throw Error("Workspace Hub checkout is missing or is a symlink", 5);
    const auto source_path = relative_path(text(lock, "source_path"), "source path");
    if (git_at(hub, {"rev-parse", "HEAD"}) != text(lock, "hub_revision"))
        throw Error("Workspace Hub checkout differs from its lock", 5);
    const auto tree = git_at(hub, {"ls-tree", "HEAD", "--", source_path.generic_string()});
    if (tree.rfind("160000 commit " + text(lock, "source_revision") + "\t", 0) != 0)
        throw Error("Workspace Hub gitlink differs from its source revision", 5);
    if (git_at(root / "source", {"rev-parse", "HEAD"}) != text(lock, "source_revision"))
        throw Error("Workspace source checkout differs from its lock", 5);
    const auto id = text(lock, "project");
    const auto published = hub / "projects" / id;
    if (read_json(root / "project.json") != read_json(published / "project.json") ||
        read_json(root / "setups.json") != read_json(published / "setups.json"))
        throw Error("Workspace project record or setup manifest differs from its pinned Hub revision", 5);
}

bool plain_trees_equal(const fs::path& left, const fs::path& right) {
    if (!fs::is_directory(left) || !fs::is_directory(right)) return false;
    const auto generated = [](const fs::path& relative) {
        for (const auto& part : relative) {
            const auto name = part.string();
            if (name == "__pycache__" || name == ".pytest_cache" ||
                (name.size() >= 9 && name.substr(name.size() - 9) == ".egg-info")) return true;
        }
        return relative.extension() == ".pyc";
    };
    std::map<fs::path, fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(left)) {
        const auto key = entry.path().lexically_relative(left);
        if (generated(key)) continue;
        if (entry.is_symlink()) return false;
        if (entry.is_directory()) continue;
        if (!entry.is_regular_file()) return false;
        files.emplace(key, entry.path());
    }
    for (const auto& entry : fs::recursive_directory_iterator(right)) {
        const auto key = entry.path().lexically_relative(right);
        if (generated(key)) continue;
        if (entry.is_symlink()) return false;
        if (entry.is_directory()) continue;
        if (!entry.is_regular_file()) return false;
        const auto found = files.find(key);
        if (found == files.end() || fs::file_size(entry) != fs::file_size(found->second)) return false;
        std::ifstream a(entry.path(), std::ios::binary), b(found->second, std::ios::binary);
        if (!std::equal(std::istreambuf_iterator<char>(a), std::istreambuf_iterator<char>(),
                        std::istreambuf_iterator<char>(b), std::istreambuf_iterator<char>())) return false;
        files.erase(found);
    }
    return files.empty();
}

void fetch(const std::string& id, const std::string& selected_dir, const std::string& requested) {
    const auto root = fs::absolute(selected_dir.empty() ? fs::current_path() : fs::path(selected_dir)).lexically_normal();
    const auto data = catalog();
    const auto project = project_from(data, id);
    const auto revision = sha(requested.empty() ? text(data, "hub_revision") : requested, "requested Hub revision");
    if (revision != text(data, "hub_revision"))
        throw Error("Requested revision is not in the published catalogue; use a published revision");
    const auto existing = fs::exists(root);
    if (existing && !fs::is_directory(root)) throw Error("Workspace path is not a directory: " + root.string());
    const std::vector<fs::path> entries{".snthub", "source", "dipl", "project.json", "setups.json"};
    if (fs::is_regular_file(root / ".snthub/lock.json")) {
        const auto current = workspace(root.string());
        if (text(current.lock, "project") != id || text(current.lock, "hub_revision") != revision)
            throw Error("Workspace already contains a different project or revision; update is not available yet");
        verify_workspace(current);
        std::cout << "Already fetched " << id << " at " << root << '\n';
        return;
    }
    for (const auto& entry : entries)
        if (fs::symlink_status(root / entry).type() != fs::file_type::not_found)
            throw Error("Workspace path already exists: " + (root / entry).string());
    fs::create_directories(root.parent_path());
    TemporaryDirectory temporary(root.parent_path(), id + "-fetch");
    const auto stage = temporary.path / "workspace";
    fs::create_directories(stage / ".snthub");
    const auto hub = stage / ".snthub/hub";
    git({"clone", "--no-checkout", text(data, "hub_repository"), hub.string()});
    git_at(hub, {"checkout", "--detach", revision});
    const auto [record, source_path] = verified_project(hub, project);
    const auto source_revision = sha(text(record, "source_revision"), "source revision");
    const auto tree = git_at(hub, {"ls-tree", "HEAD", "--", source_path.generic_string()});
    if (tree.rfind("160000 commit " + source_revision + "\t", 0) != 0)
        throw Error("Pinned Hub gitlink disagrees with its source revision", 5);
    git({"clone", "--no-checkout", text(record, "source_url"), (stage / "source").string()});
    git_at(stage / "source", {"checkout", "--detach", source_revision});
    if (git_at(stage / "source", {"rev-parse", "HEAD"}) != source_revision)
        throw Error("Source checkout differs from the pinned revision", 5);
    const auto bundle = inside(hub, relative_path(text(record.at("hub"), "adapter_package"), "adapter package"));
    for (const auto& entry : fs::recursive_directory_iterator(bundle))
        if (entry.is_symlink()) throw Error("Hub adapter bundle contains a symlink: " + entry.path().string(), 5);
    fs::copy(bundle, stage / "dipl", fs::copy_options::recursive);
    fs::copy_file(hub / "projects" / id / "project.json", stage / "project.json");
    fs::copy_file(inside(hub, relative_path(text(record.at("hub"), "setup_manifest"), "setup manifest")),
                  stage / "setups.json");
    Json lock{{"schema_version", 1}, {"materialization_version", 1},
              {"catalogue_url", environment("SNT_HUB_CATALOG_URL").empty() ? catalog_url : environment("SNT_HUB_CATALOG_URL")},
              {"catalogue_schema_version", number(data, "schema_version")},
              {"project", id}, {"hub_repository", text(data, "hub_repository")},
              {"hub_revision", revision}, {"source_url", text(record, "source_url")},
              {"source_revision", source_revision}, {"source_path", source_path.generic_string()},
              {"adapter_protocol", 1}, {"adapter_executable", text(record.at("hub"), "adapter_executable")},
              {"runtime_package", text(record.at("hub"), "runtime_package")},
              {"adapter_package", text(record.at("hub"), "adapter_package")},
              {"setup_manifest", text(record.at("hub"), "setup_manifest")}};
    write_json(stage / ".snthub/lock.json", lock);
    { std::ofstream ignore(stage / ".snthub/.gitignore"); ignore << "runtime/\n"; }
    if (!existing) fs::create_directory(root);
    std::vector<fs::path> published;
    try {
        for (const auto& entry : {fs::path("source"), fs::path("dipl"), fs::path("project.json"),
                                  fs::path("setups.json"), fs::path(".snthub")}) {
            if (fs::symlink_status(root / entry).type() != fs::file_type::not_found)
                throw Error("Workspace path appeared during fetch: " + (root / entry).string(), 5);
            fs::rename(stage / entry, root / entry);
            published.push_back(entry);
        }
    } catch (...) {
        for (const auto& entry : published) { std::error_code ignored; fs::remove_all(root / entry, ignored); }
        throw;
    }
    std::cout << "Fetched " << id << " into " << root << '\n'
              << "Hub revision: " << revision << '\n'
              << "Source revision: " << source_revision << '\n';
}

Json workspace_setups(const Workspace& current) {
    verify_workspace(current);
    const auto manifest = read_json(current.root / "setups.json");
    if (number(manifest, "schema_version") != 1 || !manifest.contains("setups") ||
        !manifest["setups"].is_object())
        throw Error("Unsupported workspace setup manifest", 5);
    return manifest["setups"];
}

void print_examples(const Json& setups) {
    for (auto item = setups.begin(); item != setups.end(); ++item) {
        const auto capability = text(item.value(), "capability");
        if (capability != "complete" && capability != "native-inputs-only" && capability != "requires-external-data")
            throw Error("Invalid capability for setup " + item.key());
        std::cout << item.key() << '\t' << capability << '\n';
    }
}

fs::path workspace_runtime(const Workspace& current) {
    const auto runtime = current.root / ".snthub/runtime";
    if (fs::is_symlink(runtime)) throw Error("Workspace adapter runtime is a symlink", 5);
    const auto lock_path = runtime / "runtime-lock.json";
    const auto executable = text(current.lock, "adapter_executable");
    if (fs::exists(runtime)) {
        if (!fs::is_regular_file(lock_path) || !fs::is_regular_file(adapter_path(runtime, executable)))
            throw Error("Incomplete workspace adapter runtime; remove .snthub/runtime before retrying", 5);
        const auto lock = read_json(lock_path);
        if (number(lock, "schema_version") != 1 || text(lock, "hub_revision") != text(current.lock, "hub_revision") ||
            text(lock, "project") != text(current.lock, "project") ||
            package_versions(runtime) != lock.at("package_versions"))
            throw Error("Workspace adapter runtime differs from its lock", 5);
        return runtime;
    }
#ifdef _WIN32
    const std::string default_python = "python";
#else
    const std::string default_python = "python3";
#endif
    const auto configured = environment("SNT_HUB_PYTHON");
    const auto python = configured.empty() ? default_python : configured;
    try {
        run({python, "-m", "venv", runtime.string()});
        const auto shared = inside(current.root / ".snthub/hub",
            relative_path(text(current.lock, "runtime_package"), "runtime package"));
        run({runtime_python(runtime).string(), "-m", "pip", "install", "--disable-pip-version-check",
             "--no-input", shared.string(), "-e", (current.root / "dipl").string()});
        if (!fs::is_regular_file(adapter_path(runtime, executable)))
            throw Error("Adapter package did not install its declared executable", 5);
        write_json(lock_path, Json{{"schema_version", 1}, {"project", text(current.lock, "project")},
                                   {"hub_revision", text(current.lock, "hub_revision")},
                                   {"package_versions", package_versions(runtime)}});
    } catch (...) {
        std::error_code ignored;
        fs::remove_all(runtime, ignored);
        throw;
    }
    return runtime;
}

int workspace_setup(const Workspace& current, const std::string& name, const std::string& output,
                    const std::string& override_file, bool inputs_only) {
    identifier(name, "example ID");
    const auto setups = workspace_setups(current);
    if (!setups.contains(name)) throw Error("Unknown setup: " + name);
    const auto capability = text(setups.at(name), "capability");
    if (capability != "complete" && !inputs_only)
        throw Error(name + " is " + capability + "; use --inputs-only");
    const auto destination = output.empty() ? current.root / "runs" / name : fs::absolute(output).lexically_normal();
    const auto canonical_root = fs::weakly_canonical(current.root);
    const auto canonical_destination = fs::weakly_canonical(destination);
    const auto relative = canonical_destination.lexically_relative(canonical_root);
    if (relative.empty() || *relative.begin() == "..")
        throw Error("Setup output must stay inside the workspace: " + destination.string());
    if (fs::symlink_status(destination).type() != fs::file_type::not_found)
        throw Error("Output already exists: " + destination.string());
    std::string override_path;
    if (!override_file.empty()) {
        const auto candidate = fs::absolute(override_file);
        if (!fs::is_regular_file(candidate)) throw Error("Override file does not exist: " + candidate.string());
        override_path = fs::canonical(candidate).string();
    }
    const bool dipl_modified = !plain_trees_equal(current.root / "dipl",
        current.root / ".snthub/hub/projects" / text(current.lock, "project") / "dipl");
    const bool source_modified = !git_at(current.root / "source", {"status", "--porcelain"}).empty();
    const auto runtime = workspace_runtime(current);
    fs::create_directories(destination.parent_path());
    TemporaryDirectory temporary(destination.parent_path(), name + "-setup");
    const auto staged = temporary.path / "result";
    std::vector<std::string> command{adapter_path(runtime, text(current.lock, "adapter_executable")).string(),
        "setup", "--bundle", (current.root / "dipl").string(), "--source", (current.root / "source").string(),
        "--setup", name, "--output", staged.string()};
    if (inputs_only) command.push_back("--inputs-only");
    if (!override_path.empty()) { command.push_back("--override-file"); command.push_back(override_path); }
    const auto result = process(command);
    if (result.status == 127) throw Error("Installed adapter executable is unavailable", 4);
    if (result.status != 0) {
        if (!result.output.empty())
            std::cerr << result.output << (result.output.back() == '\n' ? "" : "\n");
        return result.status;
    }
    const auto setup_lock_path = staged / "setup-lock.json";
    auto setup_lock = read_json(setup_lock_path);
    setup_lock["hub_revision"] = text(current.lock, "hub_revision");
    setup_lock["source_revision"] = text(current.lock, "source_revision");
    setup_lock["dipl_modified"] = dipl_modified;
    setup_lock["source_modified"] = source_modified;
    setup_lock["adapter_package_versions"] = package_versions(runtime);
    write_json(setup_lock_path, setup_lock);
    if (fs::symlink_status(destination).type() != fs::file_type::not_found)
        throw Error("Output appeared during setup: " + destination.string(), 5);
    fs::rename(staged, destination);
    std::cout << "Prepared " << name << '\n'
              << "  Output: " << destination.string() << '\n'
              << "  Setup lock: setup-lock.json (in output directory)\n";
    return 0;
}

fs::path workspace_path(const Workspace& current, const fs::path& selected, const char* label) {
    const auto resolved = fs::weakly_canonical(fs::absolute(selected));
    const auto root = fs::weakly_canonical(current.root);
    const auto relative = resolved.lexically_relative(root);
    if (relative.empty() || *relative.begin() == "..")
        throw Error(std::string(label) + " must stay inside the workspace: " + resolved.string());
    return resolved;
}

std::pair<fs::path, Json> prepared_setup(const Workspace& current, const std::string& selected) {
    const auto path = fs::path(selected);
    const auto candidate = selected.empty() ? fs::current_path() :
                           path.is_absolute() ? path : current.root / path;
    const auto directory = workspace_path(current, candidate, "Prepared setup");
    if (!fs::is_directory(directory) || !fs::is_regular_file(directory / "setup-lock.json"))
        throw Error("No prepared setup at " + directory.string());
    const auto lock = read_json(directory / "setup-lock.json");
    if (number(lock, "schema_version") != 1 || text(lock, "project") != text(current.lock, "project") ||
        text(lock, "hub_revision") != text(current.lock, "hub_revision") ||
        text(lock, "source_revision") != text(current.lock, "source_revision"))
        throw Error("Prepared setup does not match this workspace", 5);
    if (lock.contains("files")) {
        if (!lock.at("files").is_array()) throw Error("Invalid setup file list", 5);
        for (const auto& item : lock.at("files")) {
            if (!item.is_string()) throw Error("Invalid setup file path", 5);
            const auto file = inside(directory, relative_path(item.get<std::string>(), "setup file"));
            if (!fs::exists(file)) throw Error("Prepared setup is missing " + file.string(), 5);
        }
    }
    return {directory, lock};
}

Json workspace_hub_record(const Workspace& current) {
    verify_workspace(current);
    return read_json(current.root / "project.json").at("hub");
}

int workspace_build(const Workspace& current, const std::string& selected_setup,
                    const std::string& selected_profile) {
    const auto hub = workspace_hub_record(current);
    if (!hub.contains("build"))
        throw Error("This project has no reviewed build recipe; use its upstream build instructions");
    const auto& capability = hub.at("build");
    if (number(capability, "protocol") != 1) throw Error("Unsupported build protocol");
    const auto profile = selected_profile.empty() ? text(capability, "default_profile") :
                         identifier(selected_profile, "build profile");
    bool declared = false;
    for (const auto& item : capability.at("profiles"))
        if (item == profile) declared = true;
    if (!declared) throw Error("Undeclared build profile: " + profile);
    fs::path setup_directory;
    std::string setup_digest;
    const bool required = capability.at("requires_setup").get<bool>();
    if (required || !selected_setup.empty() || fs::is_regular_file(fs::current_path() / "setup-lock.json")) {
        const auto [directory, lock] = prepared_setup(current, selected_setup);
        setup_directory = directory;
        setup_digest = file_sha256(directory / "setup-lock.json");
    }
    const auto destination = workspace_path(current, current.root / "build" / profile, "Build output");
    if (fs::symlink_status(destination).type() != fs::file_type::not_found)
        throw Error("Build output already exists: " + destination.string());
    const auto runtime = workspace_runtime(current);
    fs::create_directories(destination.parent_path());
    TemporaryDirectory temporary(destination.parent_path(), profile + "-build");
    const auto staged = temporary.path / "result";
    std::vector<std::string> command{adapter_path(runtime, text(current.lock, "adapter_executable")).string(),
        "build", "--bundle", (current.root / "dipl").string(), "--source", (current.root / "source").string(),
        "--workspace", current.root.string(), "--profile", profile};
    if (!setup_directory.empty()) {
        command.push_back("--setup-dir");
        command.push_back(setup_directory.string());
    }
    command.push_back("--output");
    command.push_back(staged.string());
    const auto result = process(command, false);
    if (result.status == 127) throw Error("Adapter executable is unavailable", 4);
    if (result.status != 0) return result.status;
    if (fs::is_symlink(staged / "build-lock.json"))
        throw Error("Adapter created a symlink for its build lock", 5);
    auto lock = read_json(staged / "build-lock.json");
    if (!lock.is_object() || number(lock, "schema_version") != 1)
        throw Error("Adapter produced an invalid build lock", 5);
    text(lock, "compiler");
    if (!lock.contains("build_options") || !lock.at("build_options").is_array())
        throw Error("Adapter build lock needs a build_options array", 5);
    const auto executable = inside(staged, relative_path(text(lock, "executable"), "built executable"));
    const auto log = inside(staged, relative_path(text(lock, "build_log"), "build log"));
    if (!fs::is_regular_file(executable) || !fs::is_regular_file(log))
        throw Error("Adapter did not produce its declared executable and build log", 5);
    lock["executable"] = executable.lexically_relative(fs::weakly_canonical(staged)).generic_string();
    lock["build_log"] = log.lexically_relative(fs::weakly_canonical(staged)).generic_string();
    lock["project"] = text(current.lock, "project");
    lock["hub_revision"] = text(current.lock, "hub_revision");
    lock["source_revision"] = text(current.lock, "source_revision");
    lock["source_modified"] = !git_at(current.root / "source", {"status", "--porcelain"}).empty();
    lock["profile"] = profile;
    lock["executable_sha256"] = file_sha256(executable);
    if (!setup_digest.empty()) lock["setup_lock_sha256"] = setup_digest;
    write_json(staged / "build-lock.json", lock);
    if (fs::symlink_status(destination).type() != fs::file_type::not_found)
        throw Error("Build output appeared during build: " + destination.string(), 5);
    fs::rename(staged, destination);
    std::cout << "Built " << profile << " in " << destination << '\n';
    return 0;
}

int workspace_run(const Workspace& current, const std::string& selected_setup,
                  const std::string& selected_executable) {
    const auto hub = workspace_hub_record(current);
    if (!hub.contains("run"))
        throw Error("This project has no reviewed run recipe; setup does not imply solver execution");
    const auto& capability = hub.at("run");
    if (number(capability, "protocol") != 1) throw Error("Unsupported run protocol");
    bool local = false;
    for (const auto& item : capability.at("modes")) if (item == "local") local = true;
    if (!local) throw Error("This project does not declare a local run mode");
    const auto [setup_directory, setup_lock] = prepared_setup(current, selected_setup);
    if (text(setup_lock, "capability") != "complete")
        throw Error("This setup lacks required inputs; only complete setups can run");
    const auto run_lock_path = setup_directory / "run-lock.json";
    if (fs::symlink_status(run_lock_path).type() != fs::file_type::not_found)
        throw Error("This setup already has a run record; rerun policy is not defined");
    fs::path executable;
    std::string build_profile;
    if (!selected_executable.empty()) {
        executable = fs::canonical(selected_executable);
        if (!fs::is_regular_file(executable)) throw Error("Executable is not a regular file");
    } else {
        const auto builds = current.root / "build";
        if (fs::is_symlink(builds)) throw Error("Workspace build directory is a symlink", 5);
        if (fs::is_directory(builds)) for (const auto& item : fs::directory_iterator(builds)) {
            if (item.is_symlink() || !item.is_directory() ||
                !fs::is_regular_file(item.path() / "build-lock.json")) continue;
            const auto lock = read_json(item.path() / "build-lock.json");
            if (number(lock, "schema_version") != 1 || text(lock, "project") != text(current.lock, "project") ||
                text(lock, "hub_revision") != text(current.lock, "hub_revision") ||
                text(lock, "source_revision") != text(current.lock, "source_revision")) continue;
            if (lock.contains("setup_lock_sha256") &&
                text(lock, "setup_lock_sha256") != file_sha256(setup_directory / "setup-lock.json")) continue;
            const auto candidate = inside(item.path(), relative_path(text(lock, "executable"), "built executable"));
            if (!fs::is_regular_file(candidate) || file_sha256(candidate) != text(lock, "executable_sha256"))
                throw Error("Built executable differs from its build lock", 5);
            if (!executable.empty()) throw Error("Several matching builds exist; pass --executable PATH");
            executable = candidate;
            build_profile = text(lock, "profile");
        }
        if (executable.empty()) throw Error("No matching build; pass --executable PATH");
    }
    const auto executable_digest = file_sha256(executable);
    const auto runtime = workspace_runtime(current);
    std::vector<std::string> command{adapter_path(runtime, text(current.lock, "adapter_executable")).string(),
        "run", "--bundle", (current.root / "dipl").string(), "--source", (current.root / "source").string(),
        "--workspace", current.root.string(), "--setup-dir", setup_directory.string(),
        "--executable", executable.string()};
    const auto result = process(command, false);
    if (fs::is_symlink(run_lock_path)) throw Error("Adapter created a symlink for its run lock", 5);
    Json lock = fs::is_regular_file(run_lock_path) ? read_json(run_lock_path) : Json::object();
    if (!lock.is_object()) throw Error("Adapter produced an invalid run lock", 5);
    lock["schema_version"] = 1;
    lock["project"] = text(current.lock, "project");
    lock["hub_revision"] = text(current.lock, "hub_revision");
    lock["source_revision"] = text(current.lock, "source_revision");
    lock["source_modified"] = !git_at(current.root / "source", {"status", "--porcelain"}).empty();
    lock["setup_lock_sha256"] = file_sha256(setup_directory / "setup-lock.json");
    lock["executable"] = executable.string();
    lock["executable_sha256"] = executable_digest;
    lock["executable_source"] = build_profile.empty() ? "user-supplied" : "workspace-build";
    if (!build_profile.empty()) lock["build_profile"] = build_profile;
    lock["adapter_command"] = command;
    if (!lock.contains("command")) lock["command"] = Json::array();
    if (!lock.contains("launcher")) lock["launcher"] = "not-reported";
    if (!lock.contains("outputs")) lock["outputs"] = Json::array();
    lock["exit_status"] = result.status;
    write_json(run_lock_path, lock);
    if (result.status == 127) throw Error("Adapter executable is unavailable", 4);
    return result.status;
}

void help(const std::string& action = {}) {
    if (action == "setup") {
        std::cout << "Usage: snt hub setup EXAMPLE [--output DIR] [--override-file PATH] [--inputs-only] [--workspace DIR]\n"
                     "Prepare a run in the current workspace (default: runs/EXAMPLE).\n";
    } else if (action == "fetch") {
        std::cout << "Usage: snt hub fetch PROJECT [--dir PATH] [--revision HUB_SHA]\n"
                     "Fetch a pinned project into a local scientific workspace.\n";
    } else if (action == "build") {
        std::cout << "Usage: snt hub build [--setup DIR] [--profile NAME] [--workspace DIR]\n"
                     "Build with a reviewed project recipe, when declared by this workspace.\n";
    } else if (action == "run") {
        std::cout << "Usage: snt hub run [--setup DIR] [--executable PATH] [--workspace DIR]\n"
                     "Run a complete setup with a reviewed local project recipe.\n";
    } else {
        std::cout << "Usage: snt hub {list|fetch|examples|info|setup|build|run} [arguments]\n"
                     "Prepare pinned code examples in a local SNT Hub workspace.\n"
                     "  list             Show projects in the live catalogue\n"
                     "  fetch PROJECT    Fetch a pinned project into the current directory\n"
                     "  examples         Show the current workspace's example recipes\n"
                     "  info             Show the current workspace's lock\n"
                     "  setup EXAMPLE    Prepare a run in runs/EXAMPLE\n"
                     "  build            Use an optional reviewed project build recipe\n"
                     "  run              Use an optional reviewed local run recipe\n";
    }
}

struct Arguments {
    std::string action, target, output, override_file, revision, directory, workspace;
    std::string setup, profile, executable;
    bool inputs_only = false, show_help = false;
};

Arguments parse(int argc, char* argv[]) {
    Arguments args;
    std::vector<std::string> positionals;
    for (int i = 2; i < argc; ++i) {
        const std::string value(argv[i]);
        if (value == "-h" || value == "--help") args.show_help = true;
        else if (value == "--inputs-only") args.inputs_only = true;
        else if (value == "--output" || value == "--override-file" || value == "--revision" ||
                 value == "--dir" || value == "--workspace" || value == "--setup" ||
                 value == "--profile" || value == "--executable") {
            if (++i >= argc) throw Error(value + " requires one value");
            const std::string selected(argv[i]);
            auto* destination = value == "--output" ? &args.output :
                value == "--override-file" ? &args.override_file : value == "--revision" ? &args.revision :
                value == "--dir" ? &args.directory : value == "--workspace" ? &args.workspace :
                value == "--setup" ? &args.setup : value == "--profile" ? &args.profile : &args.executable;
            if (!destination->empty()) throw Error(value + " may be specified only once");
            *destination = selected;
        } else if (!value.empty() && value.front() == '-') throw Error("Unknown hub option: " + value);
        else positionals.push_back(value);
    }
    if (!positionals.empty()) args.action = positionals.front();
    if (positionals.size() > 1) args.target = positionals[1];
    if (positionals.size() > 2) throw Error("Too many hub command arguments");
    if (args.show_help) return args;
    if (args.action != "list" && args.action != "fetch" && args.action != "examples" &&
        args.action != "info" && args.action != "setup" && args.action != "build" && args.action != "run")
        throw Error("Unknown hub command. Use 'snt hub --help'.");
    const auto expected = args.action == "fetch" || args.action == "setup" ? 2u : 1u;
    if (positionals.size() != expected) throw Error("Unexpected hub command arguments");
    if (args.action != "setup" && (!args.output.empty() || !args.override_file.empty() || args.inputs_only))
        throw Error("Setup options require 'snt hub setup'");
    if (args.action != "fetch" && !args.revision.empty()) throw Error("--revision requires fetch");
    if (args.action != "fetch" && !args.directory.empty()) throw Error("--dir requires fetch");
    if (!args.workspace.empty() && args.action != "examples" && args.action != "info" &&
        args.action != "setup" && args.action != "build" && args.action != "run")
        throw Error("--workspace requires examples, info, setup, build, or run");
    if (!args.setup.empty() && args.action != "build" && args.action != "run")
        throw Error("--setup requires build or run");
    if (!args.profile.empty() && args.action != "build") throw Error("--profile requires build");
    if (!args.executable.empty() && args.action != "run") throw Error("--executable requires run");
    return args;
}

} // namespace

int command(int argc, char* argv[]) {
    const auto args = parse(argc, argv);
    if (args.show_help) { help(args.action); return 0; }
    if (args.action == "list") {
        const auto data = catalog();
        for (const auto& project : data.at("projects"))
            std::cout << text(project, "id") << '\t' << text(project, "name") << '\t'
                      << text(project, "source_revision") << '\n';
    } else if (args.action == "fetch") {
        fetch(args.target, args.directory, args.revision);
    } else if (args.action == "examples") {
        print_examples(workspace_setups(workspace(args.workspace)));
    } else if (args.action == "info") {
        const auto current = workspace(args.workspace);
        verify_workspace(current);
        std::cout << current.lock.dump(2) << '\n';
    } else if (args.action == "setup") {
        return workspace_setup(workspace(args.workspace), args.target, args.output,
                               args.override_file, args.inputs_only);
    } else if (args.action == "build") {
        return workspace_build(workspace(args.workspace), args.setup, args.profile);
    } else if (args.action == "run") {
        return workspace_run(workspace(args.workspace), args.setup, args.executable);
    }
    return 0;
}

} // namespace snt::hub
