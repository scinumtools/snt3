#include "storage.h"

#include "../service_support.h"

#include <chrono>
#include <fstream>
#include <regex>
#include <snt/sha256.h>
#include <sstream>

namespace snt::mcp {
    namespace fs = std::filesystem;
    namespace {
        std::int64_t now() {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        }
        void check_id(const std::string& id) {
            if (!std::regex_match(id, std::regex("artifact-[0-9]+-[0-9]+")))
                throw ServiceError("artifact.not_found", "Unknown artifact identifier.");
        }
        Json load(const fs::path& file) {
            std::ifstream input(file);
            if (!input)
                throw ServiceError("artifact.not_found", "Artifact is missing or expired; regenerate it.");
            return Json::parse(input);
        }
        std::string base64(const std::string& value) {
            constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string result;
            unsigned bits = 0;
            int available = 0;
            for (unsigned char c : value) {
                bits = (bits << 8) | c;
                available += 8;
                while (available >= 6) {
                    available -= 6;
                    result += alphabet[(bits >> available) & 63];
                }
            }
            if (available)
                result += alphabet[(bits << (6 - available)) & 63];
            while (result.size() % 4)
                result += '=';
            return result;
        }
        std::uintmax_t bytes(const fs::path& directory) {
            std::uintmax_t count = 0;
            if (fs::exists(directory))
                for (const auto& entry : fs::recursive_directory_iterator(directory))
                    if (entry.is_regular_file())
                        count += entry.file_size();
            return count;
        }
    } // namespace
    Json Storage::put_file(
        const fs::path& file, const std::string& kind, const std::string& mime, const std::string& filename
    ) const {
        if (staging_.empty())
            throw std::logic_error("Artifact writes require a worker workspace.");
        if (fs::file_size(file) > artifact_budget || bytes(staging_) + fs::file_size(file) > artifact_budget)
            throw ServiceError("artifact.limit", "Artifact exceeds the 256 MiB workspace budget; narrow the query.");
        fs::create_directories(staging_ / "artifacts");
        std::random_device random;
        std::string id;
        fs::path directory;
        do {
            id = "artifact-" + std::to_string(random()) + "-" + std::to_string(random());
            directory = staging_ / "artifacts" / id;
        } while (fs::exists(root_ / "artifacts" / id) || !fs::create_directory(directory));
        const auto destination = directory / (kind == "snapshot" ? "data.diph5" : "data.bin");
        fs::copy_file(file, destination);
        std::ifstream stream(destination, std::ios::binary);
        Sha256 hash;
        char buffer[65536];
        while (stream) {
            stream.read(buffer, sizeof(buffer));
            hash.update(buffer, static_cast<std::size_t>(stream.gcount()));
        }
        if (stream.bad())
            throw std::runtime_error("Cannot hash artifact.");
        Json metadata = {
            {"id", id},
            {"uri", "snt://artifacts/" + id},
            {"kind", kind},
            {"mimeType", mime},
            {"filename", filename},
            {"size", fs::file_size(destination)},
            {"sha256", hash.finish()},
            {"created_at", now()},
            {"expires_at", now() + 3600}
        };
        std::ofstream output(directory / "metadata.json");
        output << metadata.dump();
        if (!output)
            throw std::runtime_error("Cannot write artifact metadata.");
        return metadata;
    }
    Json Storage::put_json(const Json& value, const std::string& kind) const {
        app::TemporaryDirectory temporary(staging_);
        auto path = temporary.path / "result.json";
        {
            std::ofstream output(path);
            output << value.dump();
            if (!output)
                throw std::runtime_error("Cannot write JSON artifact.");
        }
        return put_file(path, kind, "application/json", "result.json");
    }
    Json Storage::metadata(const std::string& id) const {
        check_id(id);
        auto file = root_ / "artifacts" / id / "metadata.json";
        if (!staging_.empty() && fs::exists(staging_ / "artifacts" / id / "metadata.json"))
            file = staging_ / "artifacts" / id / "metadata.json";
        auto result = load(file);
        if (result.at("expires_at").get<std::int64_t>() <= now())
            throw ServiceError("artifact.expired", "Artifact expired; regenerate it.");
        return result;
    }
    fs::path Storage::path(const std::string& id) const {
        auto info = metadata(id);
        auto directory = root_ / "artifacts" / id;
        if (!staging_.empty() && fs::exists(staging_ / "artifacts" / id))
            directory = staging_ / "artifacts" / id;
        return directory / (info["kind"] == "snapshot" ? "data.diph5" : "data.bin");
    }
    Json Storage::read(const std::string& id, std::size_t offset, std::size_t limit) const {
        auto info = metadata(id);
        const auto size = info.at("size").get<std::size_t>();
        if (offset > size || limit == 0 || limit > chunk_limit)
            throw ServiceError(
                "argument.invalid", "Offset must be within the artifact; length must be 1..131072 bytes."
            );
        std::ifstream input(path(id), std::ios::binary);
        input.seekg(static_cast<std::streamoff>(offset));
        std::string data(std::min(limit, size - offset), '\0');
        input.read(data.data(), static_cast<std::streamsize>(data.size()));
        if (static_cast<std::size_t>(input.gcount()) != data.size())
            throw std::runtime_error("Cannot read artifact chunk.");
        const auto next = offset + data.size();
        return {
            {"artifact", info},
            {"encoding", "base64"},
            {"data", base64(data)},
            {"offset", offset},
            {"length", data.size()},
            {"next_offset", next < size ? Json(next) : Json(nullptr)}
        };
    }
    Json Storage::page(Json value, std::size_t limit, const std::string& cursor) const {
        std::size_t offset = 0;
        std::string id;
        if (!cursor.empty()) {
            const auto separator = cursor.find(':');
            if (separator == std::string::npos ||
                cursor.find_first_not_of("0123456789", separator + 1) != std::string::npos ||
                separator + 1 == cursor.size())
                throw ServiceError("cursor.invalid", "Use the next_cursor returned by dipl_list.");
            id = cursor.substr(0, separator);
            if (metadata(id)["kind"] != "listing")
                throw ServiceError("cursor.invalid", "Cursor does not reference a parameter listing.");
            offset = std::stoull(cursor.substr(separator + 1));
            value = load(path(id));
        }
        auto& items = value.at("items");
        if (offset > items.size())
            throw ServiceError("cursor.invalid", "Cursor offset is outside the listing.");
        const auto end = offset + std::min(limit, items.size() - offset);
        if (end < items.size() && id.empty())
            id = put_json(value, "listing")["id"];
        Json page = Json::array();
        for (auto i = offset; i < end; ++i)
            page.push_back(std::move(items[i]));
        value["items"] = std::move(page);
        value["truncated"] = end < value.at("total").get<std::size_t>();
        value["next_cursor"] = value["truncated"] == true ? Json(id + ":" + std::to_string(end)) : Json(nullptr);
        return value;
    }
    void Storage::prune() const {
        fs::create_directories(root_ / "artifacts");
        for (const auto& entry : fs::directory_iterator(root_ / "artifacts")) {
            if (load(entry.path() / "metadata.json").at("expires_at").get<std::int64_t>() <= now())
                fs::remove_all(entry.path());
        }
    }
    void Storage::publish() const {
        if (!fs::exists(staging_ / "artifacts"))
            return;
        if (bytes(root_ / "artifacts") + bytes(staging_ / "artifacts") > artifact_budget)
            throw ServiceError(
                "artifact.limit", "Artifact store is full (256 MiB); wait for expiry or restart the server."
            );
        for (const auto& entry : fs::directory_iterator(staging_ / "artifacts"))
            fs::rename(entry.path(), root_ / "artifacts" / entry.path().filename());
    }
} // namespace snt::mcp
