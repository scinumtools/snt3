#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <snt/dip/adapter.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <system_error>

namespace snt::dip {
    namespace fs = std::filesystem;

    namespace {
        [[noreturn]] void invalid_output(const fs::path& path, const std::string& reason) {
            throw EnvironmentException(
                "Invalid adapter output",
                "The output path `" + path.string() + "` " + reason + ".",
                "Register a distinct relative file path below the output directory.",
                __FILE__,
                __LINE__
            );
        }

        fs::path validate_relative(const fs::path& path) {
            if (path.empty() || path.is_absolute() || path.has_root_name() || path.has_root_directory())
                invalid_output(path, "must be a nonempty relative path");
            for (const auto& part : path) {
                if (part.empty() || part == "." || part == "..")
                    invalid_output(path, "contains an empty, current-directory, or parent-directory component");
            }
            const fs::path normalized = path.lexically_normal();
            if (normalized.empty() || normalized.filename().empty())
                invalid_output(path, "does not name a file");
            return normalized;
        }

        bool is_prefix(const fs::path& parent, const fs::path& child) {
            auto left = parent.begin();
            auto right = child.begin();
            while (left != parent.end() && right != child.end() && *left == *right) {
                ++left;
                ++right;
            }
            return left == parent.end();
        }

        fs::file_status checked_status(const fs::path& path) {
            std::error_code error;
            const auto status = fs::symlink_status(path, error);
            if (error && error != std::errc::no_such_file_or_directory)
                throw fs::filesystem_error("Cannot inspect adapter output", path, error);
            return status;
        }

        void validate_destinations(const fs::path& root, const std::vector<fs::path>& paths) {
            if (root.empty())
                invalid_output(root, "does not specify an output directory");
            const auto root_status = checked_status(root);
            if (fs::exists(root_status) && !fs::is_directory(root_status))
                invalid_output(root, "is not a directory");
            for (size_t i = 0; i < paths.size(); ++i) {
                for (size_t j = 0; j < i; ++j) {
                    if (is_prefix(paths[i], paths[j]) || is_prefix(paths[j], paths[i]))
                        invalid_output(paths[i], "conflicts with `" + paths[j].string() + "`");
                }
                fs::path current = root;
                for (auto part = paths[i].begin(); part != paths[i].end(); ++part) {
                    current /= *part;
                    const auto status = checked_status(current);
                    if (!fs::exists(status))
                        continue;
                    if (fs::is_symlink(status))
                        invalid_output(paths[i], "passes through a symbolic link");
                    const bool last = std::next(part) == paths[i].end();
                    if (last || !fs::is_directory(status))
                        invalid_output(paths[i], "collides with an existing filesystem entry");
                }
            }
        }

        struct StageCleanup {
            fs::path path;
            ~StageCleanup() {
                std::error_code error;
                if (!path.empty())
                    fs::remove_all(path, error);
            }
        };

        struct RootCleanup {
            fs::path path;
            bool remove_if_empty;
            ~RootCleanup() {
                if (remove_if_empty) {
                    std::error_code error;
                    fs::remove(path, error);
                }
            }
        };

        fs::path create_stage(const fs::path& root, const std::vector<fs::path>& paths) {
            static std::atomic<unsigned long long> serial{0};
            for (unsigned attempt = 0; attempt < 100; ++attempt) {
                const auto name = ".snt-adapter-stage-" +
                                  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
                                  std::to_string(serial.fetch_add(1));
                const fs::path stage = root / name;
                bool planned = false;
                for (const auto& path : paths)
                    planned = planned || *path.begin() == name;
                if (planned)
                    continue;
                std::error_code error;
                if (fs::create_directory(stage, error))
                    return stage;
                if (error && error != std::errc::file_exists)
                    throw fs::filesystem_error("Cannot create adapter staging directory", stage, error);
            }
            throw IOException(
                "Unable to stage adapter outputs",
                "Could not reserve a temporary output directory.",
                "Check the output directory permissions.",
                __FILE__,
                __LINE__
            );
        }
    } // namespace

    void AdapterContext::add_text(fs::path path, std::string content) {
        outputs_.push_back({std::move(path), Kind::Text, std::move(content), {}, {}});
    }

    void AdapterContext::add_binary(fs::path path, std::vector<std::uint8_t> content) {
        outputs_.push_back({std::move(path), Kind::Binary, {}, std::move(content), {}});
    }

    void AdapterContext::add_stream(fs::path path, StreamWriter writer) {
        if (!writer)
            invalid_output(path, "has no stream writer");
        outputs_.push_back({std::move(path), Kind::Stream, {}, {}, std::move(writer)});
    }

    std::vector<fs::path> run_adapter(
        const Environment& env, const Adapter& adapter, const fs::path& output_dir, const fs::path& snapshot
    ) {
        AdapterContext context;
        adapter.plan(env, context);

        std::vector<fs::path> relative;
        relative.reserve(context.outputs_.size() + (snapshot.empty() ? 0 : 1));
        for (const auto& output : context.outputs_)
            relative.push_back(validate_relative(output.path));
        if (!snapshot.empty())
            relative.push_back(validate_relative(snapshot));
        validate_destinations(output_dir, relative);
        if (relative.empty())
            return {};

        const bool root_existed = fs::exists(output_dir);
        fs::create_directories(output_dir);
        RootCleanup root_cleanup{output_dir, !root_existed};
        StageCleanup stage{create_stage(output_dir, relative)};
        for (size_t i = 0; i < context.outputs_.size(); ++i) {
            const auto& requested = context.outputs_[i];
            const fs::path file = stage.path / relative[i];
            fs::create_directories(file.parent_path());
            std::ofstream stream(file, std::ios::binary | std::ios::trunc);
            if (!stream)
                throw IOException(
                    "Unable to write adapter output",
                    "Could not open `" + file.string() + "`.",
                    "Check the output directory permissions.",
                    __FILE__,
                    __LINE__
                );
            switch (requested.kind) {
            case AdapterContext::Kind::Text:
                stream.write(requested.text.data(), static_cast<std::streamsize>(requested.text.size()));
                break;
            case AdapterContext::Kind::Binary:
                stream.write(
                    reinterpret_cast<const char*>(requested.binary.data()),
                    static_cast<std::streamsize>(requested.binary.size())
                );
                break;
            case AdapterContext::Kind::Stream:
                requested.writer(stream);
                break;
            }
            stream.close();
            if (!stream)
                throw IOException(
                    "Unable to write adapter output",
                    "Writing `" + file.string() + "` failed.",
                    "Check the available disk space.",
                    __FILE__,
                    __LINE__
                );
        }
        if (!snapshot.empty()) {
            const fs::path file = stage.path / relative.back();
            fs::create_directories(file.parent_path());
            env.save(file);
        }

        std::vector<fs::path> written;
        std::vector<fs::path> created_dirs;
        try {
            for (const auto& path : relative) {
                const fs::path destination = output_dir / path;
                fs::path dir = output_dir;
                for (const auto& part : path.parent_path()) {
                    dir /= part;
                    if (!fs::exists(dir)) {
                        fs::create_directory(dir);
                        created_dirs.push_back(dir);
                    }
                }
                if (fs::exists(checked_status(destination)))
                    invalid_output(path, "appeared while the adapter was writing");
                fs::rename(stage.path / path, destination);
                written.push_back(destination);
            }
        } catch (...) {
            std::error_code error;
            for (auto it = written.rbegin(); it != written.rend(); ++it)
                fs::remove(*it, error);
            for (auto it = created_dirs.rbegin(); it != created_dirs.rend(); ++it)
                fs::remove(*it, error);
            throw;
        }
        root_cleanup.remove_if_empty = false;
        return written;
    }

    std::vector<fs::path> run_adapter_project(
        const fs::path& project, const Adapter& adapter, const fs::path& output_dir, const fs::path& snapshot
    ) {
        DIP parser;
        parser.add_project(project);
        const Environment env = parser.parse();
        return run_adapter(env, adapter, output_dir, snapshot);
    }

    std::vector<fs::path> run_adapter_snapshot(
        const fs::path& input, const Adapter& adapter, const fs::path& output_dir, const fs::path& snapshot
    ) {
        Environment env;
        env.load(input);
        return run_adapter(env, adapter, output_dir, snapshot);
    }
} // namespace snt::dip
