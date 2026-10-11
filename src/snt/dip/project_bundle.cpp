#include <snt/dip/project_bundle.h>

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace snt::dip {
namespace {

const std::filesystem::path bundle_root =
    (std::filesystem::current_path() / "__snt_project_bundle__").lexically_normal();

[[noreturn]] void invalid_bundle(const std::string& details) {
    throw IOException("Invalid project bundle", details,
                      "Include every referenced local file under the DIPfile directory.", __FILE__, __LINE__);
}

std::string checked_relative(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute()) invalid_bundle("Bundle paths must be nonempty and relative.");
    const std::string spelling = path.generic_string();
    if (spelling.find('\\') != std::string::npos)
        invalid_bundle("Bundle paths must use forward slashes: `" + spelling + "`.");
    for (const auto& part : path)
        if (part == "." || part == "..")
            invalid_bundle("Bundle path `" + spelling + "` cannot contain `.` or `..` segments.");
    if (path.filename().empty()) invalid_bundle("Bundle path `" + spelling + "` must name a file.");
    return path.lexically_normal().generic_string();
}

std::string relative_to_root(const std::filesystem::path& path, const std::filesystem::path& root) {
    const auto relative = path.lexically_relative(root);
    if (relative.empty() || relative.is_absolute()) invalid_bundle("File `" + path.string() + "` is outside the project bundle.");
    return checked_relative(relative);
}

bool valid_utf8(const std::string& text) {
    for (size_t i = 0; i < text.size();) {
        const auto first = static_cast<unsigned char>(text[i]);
        if (first == 0) return false;
        if (first < 0x80) { ++i; continue; }
        size_t continuation = 0;
        if (first >= 0xc2 && first <= 0xdf) continuation = 1;
        else if (first >= 0xe0 && first <= 0xef) continuation = 2;
        else if (first >= 0xf0 && first <= 0xf4) continuation = 3;
        else return false;
        if (i + continuation >= text.size()) return false;
        const auto second = static_cast<unsigned char>(text[i + 1]);
        if ((second & 0xc0) != 0x80 ||
            (first == 0xe0 && second < 0xa0) || (first == 0xed && second >= 0xa0) ||
            (first == 0xf0 && second < 0x90) || (first == 0xf4 && second >= 0x90)) return false;
        for (size_t offset = 2; offset <= continuation; ++offset)
            if ((static_cast<unsigned char>(text[i + offset]) & 0xc0) != 0x80) return false;
        i += continuation + 1;
    }
    return true;
}

void json_string(std::ostream& out, const std::string& input) {
    out << '"';
    for (const unsigned char ch : input) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20) {
                out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned>(ch)
                    << std::dec << std::setfill(' ');
            } else out << static_cast<char>(ch);
        }
    }
    out << '"';
}

} // namespace

std::string read_project_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw IOException("File not found", "The file `" + path.string() + "` could not be opened.",
                                 "Check whether the file exists and is readable.", __FILE__, __LINE__);
    std::ostringstream contents;
    contents << file.rdbuf();
    if (file.bad()) throw IOException("File read failed", "The file `" + path.string() + "` could not be read.",
                                     "Check the source file and try again.", __FILE__, __LINE__);
    return contents.str();
}

void DIP::add_project(const ProjectBundle& bundle) {
    const std::string entry = checked_relative(bundle.entry);
    if (std::filesystem::path(entry).has_parent_path())
        invalid_bundle("The DIPfile entry must be at the root of the project bundle.");
    auto files = std::make_shared<std::map<std::string, std::string>>();
    for (const auto& file : bundle.files) {
        const auto path = checked_relative(file.path);
        if (!files->emplace(path, file.text).second)
            invalid_bundle("Bundle contains duplicate file `" + path + "`.");
    }
    if (files->find(entry) == files->end()) invalid_bundle("Bundle is missing its DIPfile `" + entry + "`.");
    for (const auto& [path, text] : *files)
        if (!valid_utf8(text)) invalid_bundle("Bundled file `" + path + "` is not UTF-8 text.");
    set_project_file_reader([files](const std::filesystem::path& requested) -> std::string {
        const auto absolute = (requested.is_absolute() ? requested : bundle_root / requested).lexically_normal();
        const auto path = relative_to_root(absolute, bundle_root);
        const auto found = files->find(path);
        if (found == files->end())
            throw IOException("Missing bundled source", "The DIPfile requires `" + path + "`, which is not in the bundle.",
                              "Package the referenced file with the project.", __FILE__, __LINE__);
        return found->second;
    });
    add_project(bundle_root / entry);
}

ProjectBundle package_project(const std::filesystem::path& project_file,
                              const std::vector<std::filesystem::path>& additional_files) {
    const auto absolute = std::filesystem::canonical(project_file);
    const auto root = absolute.parent_path();
    ProjectBundle bundle;
    bundle.entry = absolute.filename().generic_string();
    std::map<std::string, std::string> collected;
    DIP parser;
    parser.set_project_file_reader([&](const std::filesystem::path& requested) {
        const auto logical = (requested.is_absolute() ? requested : std::filesystem::absolute(requested)).lexically_normal();
        const auto path = relative_to_root(logical, root);
        const auto resolved = std::filesystem::weakly_canonical(logical);
        relative_to_root(resolved, root); // Reject symlinks escaping the bundle directory.
        auto found = collected.find(path);
        if (found != collected.end()) return found->second;
        const auto contents = read_project_file(resolved);
        if (!valid_utf8(contents)) invalid_bundle("Source file `" + resolved.string() + "` is not UTF-8 text.");
        collected.emplace(path, contents);
        return contents;
    });
    parser.add_project(absolute);
    parser.parse(); // Also discovers files opened by $source during evaluation.
    for (const auto& file : additional_files) {
        if (file.is_absolute()) invalid_bundle("Additional bundle files must use paths relative to the DIPfile directory.");
        const auto path = checked_relative(file);
        const auto resolved = std::filesystem::weakly_canonical(root / path);
        relative_to_root(resolved, root);
        if (collected.find(path) == collected.end()) {
            const auto contents = read_project_file(resolved);
            if (!valid_utf8(contents)) invalid_bundle("Source file `" + resolved.string() + "` is not UTF-8 text.");
            collected.emplace(path, contents);
        }
    }
    for (auto& [path, text] : collected) bundle.files.push_back({path, std::move(text)});
    return bundle;
}

std::string project_bundle_json(const ProjectBundle& bundle) {
    std::ostringstream out;
    out << "{\"format\":\"snt-dipl-project-bundle/1\",\"entry\":";
    json_string(out, bundle.entry);
    out << ",\"files\":[";
    for (size_t i = 0; i < bundle.files.size(); ++i) {
        if (i) out << ',';
        out << "{\"path\":";
        json_string(out, bundle.files[i].path);
        out << ",\"text\":";
        json_string(out, bundle.files[i].text);
        out << '}';
    }
    out << "]}";
    return out.str();
}

} // namespace snt::dip
