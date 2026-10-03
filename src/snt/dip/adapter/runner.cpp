#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <fstream>
#include <set>
#include <snt/dip/adapter.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <system_error>

namespace snt::dip {
    namespace fs = std::filesystem;

    namespace {
        constexpr const char* manifest_name = ".snt-adapter-manifest";
        constexpr const char* manifest_header = "SNT-ADAPTER-MANIFEST-1\n";

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
            const auto& native = path.native();
            if (std::find(native.begin(), native.end(), fs::path::value_type{}) != native.end())
                invalid_output(path, "contains a null character");
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

        void validate_path_components(const fs::path& root, const fs::path& path) {
            fs::path current = root;
            for (auto part = path.begin(); part != path.end(); ++part) {
                current /= *part;
                const auto status = checked_status(current);
                if (!fs::exists(status))
                    continue;
                if (fs::is_symlink(status))
                    invalid_output(path, "passes through a symbolic link");
                const bool last = std::next(part) == path.end();
                if (last ? !fs::is_regular_file(status) : !fs::is_directory(status))
                    invalid_output(path, "collides with a non-file entry or passes through a non-directory entry");
            }
        }

        [[noreturn]] void invalid_manifest(const fs::path& path) {
            throw EnvironmentException(
                "Invalid adapter manifest",
                "The manifest `" + path.string() + "` is malformed or contains unsafe paths.",
                "Remove or repair the manifest before synchronizing outputs.",
                __FILE__,
                __LINE__
            );
        }

        std::vector<fs::path> read_manifest(const fs::path& root) {
            const fs::path file = root / manifest_name;
            const auto status = checked_status(file);
            if (!fs::exists(status))
                return {};
            if (!fs::is_regular_file(status) || fs::is_symlink(status))
                invalid_manifest(file);
            std::ifstream in(file, std::ios::binary);
            std::string header;
            if (!std::getline(in, header) || header + "\n" != manifest_header)
                invalid_manifest(file);
            std::vector<fs::path> paths;
            std::set<fs::path> unique;
            std::string size_line;
            while (std::getline(in, size_line)) {
                if (size_line.empty() || size_line.size() > 7 ||
                    !std::all_of(size_line.begin(), size_line.end(), [](unsigned char ch) { return ch >= '0' && ch <= '9'; }))
                    invalid_manifest(file);
                const auto length = static_cast<size_t>(std::stoul(size_line));
                if (length == 0 || length > 1000000 || paths.size() >= 100000)
                    invalid_manifest(file);
                std::string bytes(length, '\0');
                if (!in.read(bytes.data(), static_cast<std::streamsize>(length)) || in.get() != '\n')
                    invalid_manifest(file);
                fs::path path;
                try {
                    path = validate_relative(fs::u8path(bytes));
                } catch (const EnvironmentException&) {
                    invalid_manifest(file);
                }
                if (*path.begin() == manifest_name || !unique.insert(path).second)
                    invalid_manifest(file);
                for (const auto& existing : paths)
                    if (is_prefix(path, existing) || is_prefix(existing, path))
                        invalid_manifest(file);
                paths.push_back(std::move(path));
            }
            if (!in.eof())
                invalid_manifest(file);
            return paths;
        }

        void write_manifest(const fs::path& file, const std::vector<fs::path>& paths) {
            std::ofstream out(file, std::ios::binary | std::ios::trunc);
            out << manifest_header;
            for (const auto& path : paths) {
                const std::string bytes = path.generic_u8string();
                out << bytes.size() << '\n';
                out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
                out.put('\n');
            }
            out.close();
            if (!out)
                throw IOException(
                    "Unable to write adapter manifest",
                    "Writing `" + file.string() + "` failed.",
                    "Check the available disk space.",
                    __FILE__,
                    __LINE__
                );
        }

        void validate_destinations(
            const fs::path& root, const std::vector<fs::path>& paths, ExistingOutputPolicy policy
        ) {
            if (root.empty())
                invalid_output(root, "does not specify an output directory");
            const auto root_status = checked_status(root);
            if (fs::is_symlink(root_status))
                invalid_output(root, "is a symbolic link");
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
                    if (last) {
                        if (policy == ExistingOutputPolicy::Reject || !fs::is_regular_file(status))
                            invalid_output(paths[i], "collides with an existing filesystem entry");
                    } else if (!fs::is_directory(status)) {
                        invalid_output(paths[i], "passes through a non-directory entry");
                    }
                }
            }
        }

        void validate_sync_destinations(
            const fs::path& root, const std::vector<fs::path>& paths, const std::vector<fs::path>& previous
        ) {
            if (root.empty())
                invalid_output(root, "does not specify an output directory");
            const auto root_status = checked_status(root);
            if (fs::is_symlink(root_status) || (fs::exists(root_status) && !fs::is_directory(root_status)))
                invalid_output(root, "is not a directory or is a symbolic link");
            const std::set<fs::path> current(paths.begin(), paths.end());
            const std::set<fs::path> stale = [&] {
                std::set<fs::path> result;
                for (const auto& path : previous)
                    if (!current.count(path))
                        result.insert(path);
                return result;
            }();
            for (const auto& path : previous) {
                if (*path.begin() == manifest_name)
                    invalid_manifest(root / manifest_name);
                validate_path_components(root, path);
            }
            for (size_t i = 0; i < paths.size(); ++i) {
                const auto& path = paths[i];
                if (*path.begin() == manifest_name)
                    invalid_output(path, "uses the reserved adapter manifest path");
                for (size_t j = 0; j < i; ++j)
                    if (is_prefix(path, paths[j]) || is_prefix(paths[j], path))
                        invalid_output(path, "conflicts with `" + paths[j].string() + "`");
                fs::path current_path = root;
                fs::path relative_path;
                for (auto part = path.begin(); part != path.end(); ++part) {
                    current_path /= *part;
                    relative_path /= *part;
                    const auto status = checked_status(current_path);
                    if (!fs::exists(status))
                        break;
                    if (fs::is_symlink(status))
                        invalid_output(path, "passes through a symbolic link");
                    const bool last = std::next(part) == path.end();
                    if (!last) {
                        if (fs::is_regular_file(status) && stale.count(relative_path))
                            break;
                        if (!fs::is_directory(status))
                            invalid_output(path, "passes through a non-directory entry");
                    } else if (fs::is_directory(status)) {
                        for (const auto& entry : fs::recursive_directory_iterator(current_path)) {
                            const auto entry_status = checked_status(entry.path());
                            if (fs::is_directory(entry_status))
                                continue;
                            const auto entry_relative = entry.path().lexically_relative(root);
                            if (!fs::is_regular_file(entry_status) || !stale.count(entry_relative))
                                invalid_output(path, "would replace a directory containing unregistered entries");
                        }
                    } else if (!fs::is_regular_file(status)) {
                        invalid_output(path, "collides with a non-file entry");
                    }
                }
            }
            const auto manifest_status = checked_status(root / manifest_name);
            if (fs::exists(manifest_status) && (!fs::is_regular_file(manifest_status) || fs::is_symlink(manifest_status)))
                invalid_manifest(root / manifest_name);
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
        const Environment& env, const Adapter& adapter, const fs::path& output_dir, const fs::path& snapshot,
        ExistingOutputPolicy policy
    ) {
        AdapterContext context;
        adapter.plan(env, context);

        std::vector<fs::path> relative;
        relative.reserve(context.outputs_.size() + (snapshot.empty() ? 0 : 1));
        for (const auto& output : context.outputs_)
            relative.push_back(validate_relative(output.path));
        if (!snapshot.empty())
            relative.push_back(validate_relative(snapshot));
        const bool sync = policy == ExistingOutputPolicy::SyncRegistered;
        std::vector<fs::path> previous;
        if (sync) {
            if (output_dir.empty())
                invalid_output(output_dir, "does not specify an output directory");
            const auto status = checked_status(output_dir);
            if (fs::is_symlink(status) || (fs::exists(status) && !fs::is_directory(status)))
                invalid_output(output_dir, "is not a directory or is a symbolic link");
            previous = read_manifest(output_dir);
            validate_sync_destinations(output_dir, relative, previous);
        } else {
            validate_destinations(output_dir, relative, policy);
        }
        if (relative.empty() && !sync)
            return {};

        const bool root_existed = fs::exists(output_dir);
        fs::create_directories(output_dir);
        RootCleanup root_cleanup{output_dir, !root_existed};
        StageCleanup stage{create_stage(output_dir, relative)};
        for (size_t i = 0; i < context.outputs_.size(); ++i) {
            const auto& requested = context.outputs_[i];
            const fs::path file = stage.path / "new" / relative[i];
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
            const fs::path file = stage.path / "new" / relative.back();
            fs::create_directories(file.parent_path());
            env.save(file);
        }
        if (sync) {
            fs::create_directories(stage.path / "new");
            write_manifest(stage.path / "new" / manifest_name, relative);
        }

        struct PublishedOutput {
            fs::path destination;
            fs::path backup;
            bool backed_up = false;
            bool published = false;
        };
        std::vector<PublishedOutput> journal;
        journal.reserve(relative.size() + previous.size() + (sync ? 1 : 0));
        std::vector<fs::path> written;
        std::vector<fs::path> created_dirs;
        std::vector<fs::path> removed_dirs;
        try {
            // A stream writer may have changed a destination while staging.
            if (sync) {
                if (read_manifest(output_dir) != previous)
                    invalid_manifest(output_dir / manifest_name);
                validate_sync_destinations(output_dir, relative, previous);
                const std::set<fs::path> current(relative.begin(), relative.end());
                for (const auto& path : previous) {
                    if (current.count(path))
                        continue;
                    const fs::path destination = output_dir / path;
                    if (!fs::exists(checked_status(destination)))
                        continue;
                    journal.push_back({destination, stage.path / "old" / path});
                    auto& entry = journal.back();
                    fs::create_directories(entry.backup.parent_path());
                    fs::rename(destination, entry.backup);
                    entry.backed_up = true;
                }
                for (const auto& path : relative) {
                    const fs::path destination = output_dir / path;
                    if (!fs::is_directory(checked_status(destination)))
                        continue;
                    std::vector<fs::path> directories{destination};
                    for (const auto& entry : fs::recursive_directory_iterator(destination)) {
                        if (!fs::is_directory(checked_status(entry.path())))
                            invalid_output(path, "would replace a directory containing unregistered entries");
                        directories.push_back(entry.path());
                    }
                    std::sort(directories.begin(), directories.end(), [](const auto& a, const auto& b) {
                        return std::distance(a.begin(), a.end()) > std::distance(b.begin(), b.end());
                    });
                    for (const auto& dir : directories) {
                        fs::remove(dir);
                        removed_dirs.push_back(dir);
                    }
                }
            } else {
                validate_destinations(output_dir, relative, policy);
            }
            for (const auto& path : relative) {
                const fs::path destination = output_dir / path;
                fs::path dir = output_dir;
                for (const auto& part : path.parent_path()) {
                    dir /= part;
                    const auto status = checked_status(dir);
                    if (!fs::exists(status)) {
                        fs::create_directory(dir);
                        created_dirs.push_back(dir);
                    } else if (!fs::is_directory(status) || fs::is_symlink(status)) {
                        invalid_output(path, "passes through a non-directory entry or symbolic link");
                    }
                }
                const auto status = checked_status(destination);
                if (fs::is_symlink(status) || (fs::exists(status) && !fs::is_regular_file(status)))
                    invalid_output(path, "collides with an existing filesystem entry");
                if (fs::exists(status) && policy == ExistingOutputPolicy::Reject)
                    invalid_output(path, "appeared while the adapter was writing");
                validate_path_components(stage.path / "new", path);

                journal.push_back({destination, stage.path / "old" / path});
                auto& entry = journal.back();
                if (fs::exists(status)) {
                    fs::create_directories(entry.backup.parent_path());
                    fs::rename(destination, entry.backup);
                    entry.backed_up = true;
                }
                fs::rename(stage.path / "new" / path, destination);
                entry.published = true;
                written.push_back(destination);
            }
            if (sync) {
                const fs::path destination = output_dir / manifest_name;
                journal.push_back({destination, stage.path / "old" / manifest_name});
                auto& entry = journal.back();
                if (fs::exists(checked_status(destination))) {
                    fs::rename(destination, entry.backup);
                    entry.backed_up = true;
                }
                fs::rename(stage.path / "new" / manifest_name, destination);
                entry.published = true;
            }
        } catch (...) {
            const auto original_error = std::current_exception();
            std::error_code rollback_error;
            for (auto it = journal.rbegin(); it != journal.rend(); ++it) {
                std::error_code error;
                if (it->published)
                    fs::remove(it->destination, error);
                if (!rollback_error && error)
                    rollback_error = error;
            }
            for (auto it = created_dirs.rbegin(); it != created_dirs.rend(); ++it) {
                std::error_code error;
                fs::remove(*it, error);
                if (!rollback_error && error)
                    rollback_error = error;
            }
            for (auto it = removed_dirs.rbegin(); it != removed_dirs.rend(); ++it) {
                std::error_code error;
                fs::create_directory(*it, error);
                if (!rollback_error && error)
                    rollback_error = error;
            }
            for (auto it = journal.rbegin(); it != journal.rend(); ++it) {
                std::error_code error;
                if (it->backed_up) {
                    fs::rename(it->backup, it->destination, error);
                    if (!rollback_error && error)
                        rollback_error = error;
                }
            }
            if (rollback_error) {
                // Keep any backup that could not be restored for manual recovery.
                const fs::path recovery_dir = stage.path;
                stage.path.clear();
                throw fs::filesystem_error(
                    "Cannot restore adapter outputs after publication failed; backups remain in the staging directory",
                    recovery_dir, rollback_error
                );
            }
            std::rethrow_exception(original_error);
        }
        root_cleanup.remove_if_empty = false;
        return written;
    }

    std::vector<fs::path> run_adapter_project(
        const fs::path& project, const Adapter& adapter, const fs::path& output_dir, const fs::path& snapshot,
        ExistingOutputPolicy policy
    ) {
        DIP parser;
        parser.add_project(project);
        const Environment env = parser.parse();
        return run_adapter(env, adapter, output_dir, snapshot, policy);
    }

    std::vector<fs::path> run_adapter_snapshot(
        const fs::path& input, const Adapter& adapter, const fs::path& output_dir, const fs::path& snapshot,
        ExistingOutputPolicy policy
    ) {
        Environment env;
        env.load(input);
        return run_adapter(env, adapter, output_dir, snapshot, policy);
    }
} // namespace snt::dip
