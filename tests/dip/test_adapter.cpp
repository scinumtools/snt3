#include "pch_tests.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <snt/dip/adapter.h>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <sstream>

using namespace snt;
namespace fs = std::filesystem;

namespace {
    struct Workspace {
        fs::path path =
            fs::temp_directory_path() /
            ("snt-adapter-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        Workspace() { fs::create_directories(path); }
        ~Workspace() {
            std::error_code error;
            fs::remove_all(path, error);
        }
        void write(const fs::path& name, const std::string& value) const {
            std::ofstream out(path / name, std::ios::binary);
            out << value;
        }
    };

    std::string read(const fs::path& path) {
        std::ifstream in(path, std::ios::binary);
        std::ostringstream out;
        out << in.rdbuf();
        return out.str();
    }

    struct ExampleAdapter final : dip::Adapter {
        void plan(const dip::Environment& env, dip::AdapterContext& context) const override {
            const auto steps = env["run.steps"].as<int64_t>();
            context.add_text("native/control.in", "STEPS=" + std::to_string(steps) + "\n");
            context.add_binary("native/marker.bin", {0, 1, 255});
            context.add_stream("series/steps.dat", [steps](std::ostream& out) {
                for (int64_t index = 0; index < steps; ++index)
                    out << index << '\n';
            });
        }
    };

    struct CallbackAdapter final : dip::Adapter {
        std::function<void(dip::AdapterContext&)> callback;
        void plan(const dip::Environment&, dip::AdapterContext& context) const override { callback(context); }
    };
} // namespace

TEST(Adapter, ProjectAndSnapshotProduceEquivalentTextBinaryAndStreamedFiles) {
    Workspace work;
    work.write("parameters.dip", "run\n  steps int = 3\n");
    work.write("DIPfile", "code[]\n  file = \"parameters.dip\"\n");
    ExampleAdapter adapter;

    const auto project_files =
        dip::run_adapter_project(work.path / "DIPfile", adapter, work.path / "project", "run.diph5");
    ASSERT_EQ(project_files.size(), 4);
    EXPECT_EQ(read(work.path / "project/native/control.in"), "STEPS=3\n");
    EXPECT_EQ(read(work.path / "project/native/marker.bin"), std::string("\0\1\xff", 3));
    EXPECT_EQ(read(work.path / "project/series/steps.dat"), "0\n1\n2\n");
    EXPECT_TRUE(fs::exists(work.path / "project/run.diph5"));

    const auto loaded_files = dip::run_adapter_snapshot(work.path / "project/run.diph5", adapter, work.path / "loaded");
    ASSERT_EQ(loaded_files.size(), 3);
    for (const auto& path : {"native/control.in", "native/marker.bin", "series/steps.dat"})
        EXPECT_EQ(read(work.path / "project" / path), read(work.path / "loaded" / path));

    dip::DIP parser;
    parser.add_project(work.path / "DIPfile");
    const auto env = parser.parse();
    EXPECT_EQ(dip::run_adapter(env, adapter, work.path / "existing").size(), 3);
    EXPECT_EQ(read(work.path / "existing/native/control.in"), "STEPS=3\n");
}

TEST(Adapter, RejectsUnsafeAndConflictingPathsBeforeWriting) {
    Workspace work;
    dip::DIP parser;
    parser.add_string("value int = 1\n");
    const auto env = parser.parse();
    int streamed = 0;
    CallbackAdapter adapter;

    const std::vector<std::pair<std::string, std::string>> conflicts = {
        {"same.dat", "same.dat"},
        {"a", "a/b"},
        {"../escape", "safe.dat"},
        {"/absolute.dat", "safe.dat"},
        {"sub/./file", "safe.dat"}
    };
    for (size_t i = 0; i < conflicts.size(); ++i) {
        SCOPED_TRACE(i);
        adapter.callback = [&, i](dip::AdapterContext& context) {
            context.add_text(conflicts[i].first, "one");
            context.add_stream(conflicts[i].second, [&](std::ostream& out) {
                ++streamed;
                out << "two";
            });
        };
        const auto output = work.path / ("invalid-" + std::to_string(i));
        EXPECT_THROW(dip::run_adapter(env, adapter, output), dip::EnvironmentException);
        EXPECT_FALSE(fs::exists(output));
    }
    EXPECT_EQ(streamed, 0);

    adapter.callback = [](dip::AdapterContext& context) {
        context.add_text("run.diph5", "collision");
    };
    EXPECT_THROW(
        dip::run_adapter(env, adapter, work.path / "snapshot-collision", "run.diph5"), dip::EnvironmentException
    );

    fs::create_directories(work.path / "existing");
    work.write("existing/control.in", "original");
    adapter.callback = [](dip::AdapterContext& context) {
        context.add_text("control.in", "replacement");
    };
    EXPECT_THROW(dip::run_adapter(env, adapter, work.path / "existing"), dip::EnvironmentException);
    EXPECT_EQ(read(work.path / "existing/control.in"), "original");
}

TEST(Adapter, StreamFailureLeavesNoPublishedOutput) {
    Workspace work;
    dip::DIP parser;
    parser.add_string("value int = 1\n");
    const auto env = parser.parse();
    CallbackAdapter adapter;
    adapter.callback = [](dip::AdapterContext& context) {
        context.add_text("first.txt", "complete");
        context.add_stream("second.bin", [](std::ostream& out) {
            out << "partial";
            throw std::runtime_error("adapter stream failed");
        });
    };
    EXPECT_THROW(dip::run_adapter(env, adapter, work.path / "failed"), std::runtime_error);
    EXPECT_FALSE(fs::exists(work.path / "failed/first.txt"));
    EXPECT_FALSE(fs::exists(work.path / "failed/second.bin"));

    adapter.callback = [](dip::AdapterContext&) {
    };
    EXPECT_TRUE(dip::run_adapter(env, adapter, work.path / "empty").empty());
    EXPECT_FALSE(fs::exists(work.path / "empty"));
}

TEST(Adapter, ReplaceRegisteredRegeneratesOnlyPlannedFilesAndSnapshot) {
    Workspace work;
    dip::DIP parser;
    parser.add_string("run\n  steps int = 2\n");
    const auto first = parser.parse();
    ExampleAdapter adapter;
    const auto output = work.path / "output";
    const auto policy = dip::ExistingOutputPolicy::ReplaceRegistered;

    ASSERT_EQ(dip::run_adapter(first, adapter, output, "run.diph5").size(), 4);
    const auto old_snapshot = read(output / "run.diph5");
    work.write("output/unrelated.txt", "keep me");
    work.write("output/native/control.in", "old control");
    EXPECT_THROW(dip::run_adapter(first, adapter, output, "run.diph5"), dip::EnvironmentException);

    dip::DIP changed_parser;
    changed_parser.add_string("run\n  steps int = 4\n");
    const auto changed = changed_parser.parse();
    const auto written = dip::run_adapter(changed, adapter, output, "run.diph5", policy);
    ASSERT_EQ(written.size(), 4);
    EXPECT_EQ(read(output / "native/control.in"), "STEPS=4\n");
    EXPECT_EQ(read(output / "series/steps.dat"), "0\n1\n2\n3\n");
    EXPECT_EQ(read(output / "unrelated.txt"), "keep me");
    EXPECT_NE(read(output / "run.diph5"), old_snapshot);

    EXPECT_EQ(dip::run_adapter(changed, adapter, output, "run.diph5", policy).size(), 4);
    EXPECT_EQ(read(output / "unrelated.txt"), "keep me");
}

TEST(Adapter, ReplaceRegisteredRejectsDirectoryFileAndSymlinkConflicts) {
    Workspace work;
    dip::DIP parser;
    parser.add_string("value int = 1\n");
    const auto env = parser.parse();
    CallbackAdapter adapter;
    const auto output = work.path / "output";
    fs::create_directories(output / "directory");
    work.write("output/parent", "original parent");
    const auto policy = dip::ExistingOutputPolicy::ReplaceRegistered;

    adapter.callback = [](dip::AdapterContext& context) { context.add_text("directory", "new"); };
    EXPECT_THROW(dip::run_adapter(env, adapter, output, policy), dip::EnvironmentException);
    EXPECT_TRUE(fs::is_directory(output / "directory"));

    adapter.callback = [](dip::AdapterContext& context) { context.add_text("parent/child", "new"); };
    EXPECT_THROW(dip::run_adapter(env, adapter, output, policy), dip::EnvironmentException);
    EXPECT_EQ(read(output / "parent"), "original parent");

    std::error_code error;
    fs::create_symlink(output / "parent", output / "link", error);
    if (!error) {
        adapter.callback = [](dip::AdapterContext& context) { context.add_text("link", "new"); };
        EXPECT_THROW(dip::run_adapter(env, adapter, output, policy), dip::EnvironmentException);
        EXPECT_TRUE(fs::is_symlink(fs::symlink_status(output / "link")));
        adapter.callback = [](dip::AdapterContext& context) { context.add_text("link/child", "new"); };
        EXPECT_THROW(dip::run_adapter(env, adapter, output, policy), dip::EnvironmentException);
    }
}

TEST(Adapter, FailedStagedWritePreservesExistingFiles) {
    Workspace work;
    dip::DIP parser;
    parser.add_string("value int = 1\n");
    const auto env = parser.parse();
    const auto output = work.path / "output";
    fs::create_directories(output);
    work.write("output/first.txt", "old first");
    work.write("output/unrelated.txt", "unrelated");
    CallbackAdapter adapter;
    adapter.callback = [](dip::AdapterContext& context) {
        context.add_text("first.txt", "new first");
        context.add_stream("second.txt", [](std::ostream& out) {
            out << "partial";
            throw std::runtime_error("stream failed");
        });
    };
    EXPECT_THROW(dip::run_adapter(env, adapter, output, dip::ExistingOutputPolicy::ReplaceRegistered),
                 std::runtime_error);
    EXPECT_EQ(read(output / "first.txt"), "old first");
    EXPECT_EQ(read(output / "unrelated.txt"), "unrelated");
    EXPECT_FALSE(fs::exists(output / "second.txt"));
}

TEST(Adapter, PublicationFailureRestoresFilesAlreadyReplaced) {
    Workspace work;
    dip::DIP parser;
    parser.add_string("value int = 1\n");
    const auto env = parser.parse();
    const auto output = work.path / "output";
    fs::create_directories(output);
    work.write("output/first.txt", "old first");
    work.write("output/second.txt", "old second");
    work.write("output/unrelated.txt", "unrelated");
    CallbackAdapter adapter;
    adapter.callback = [&](dip::AdapterContext& context) {
        context.add_text("first.txt", "new first");
        context.add_text("second.txt", "new second");
        context.add_stream("trigger.txt", [&](std::ostream& out) {
            bool removed = false;
            for (const auto& entry : fs::directory_iterator(output)) {
                if (entry.path().filename().string().find(".snt-adapter-stage-") == 0) {
                    removed = fs::remove(entry.path() / "new/second.txt");
                    break;
                }
            }
            if (!removed)
                throw std::runtime_error("test could not remove staged output");
            out << "ready";
        });
    };
    EXPECT_THROW(dip::run_adapter(env, adapter, output, dip::ExistingOutputPolicy::ReplaceRegistered),
                 fs::filesystem_error);
    EXPECT_EQ(read(output / "first.txt"), "old first");
    EXPECT_EQ(read(output / "second.txt"), "old second");
    EXPECT_EQ(read(output / "unrelated.txt"), "unrelated");
    EXPECT_FALSE(fs::exists(output / "trigger.txt"));
}
