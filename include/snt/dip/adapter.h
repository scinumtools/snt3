#ifndef SNT_DIP_ADAPTER_H
#define SNT_DIP_ADAPTER_H

#include <cstdint>
#include <filesystem>
#include <functional>
#include <ostream>
#include <string>
#include <vector>

namespace snt::dip {
    class Environment;

    /** How a runner handles files already present at registered output paths. */
    enum class ExistingOutputPolicy {
        Reject,            ///< Reject every existing destination (default).
        ReplaceRegistered  ///< Replace regular files registered for this run only.
    };

    /** Collects files requested by one adapter run. Paths are relative to the output directory. */
    class AdapterContext {
      public:
        using StreamWriter = std::function<void(std::ostream&)>;

        /** Register UTF-8 or other application-defined text without changing its contents. */
        void add_text(std::filesystem::path path, std::string content);
        /** Register raw bytes without encoding or newline conversion. */
        void add_binary(std::filesystem::path path, std::vector<std::uint8_t> content);
        /** The writer is called once, after all output paths have been validated. */
        void add_stream(std::filesystem::path path, StreamWriter writer);

      private:
        enum class Kind { Text, Binary, Stream };
        struct Output {
            std::filesystem::path path;
            Kind kind;
            std::string text;
            std::vector<std::uint8_t> binary;
            StreamWriter writer;
        };
        std::vector<Output> outputs_;

        friend std::vector<std::filesystem::path> run_adapter(
            const Environment&, const class Adapter&, const std::filesystem::path&, const std::filesystem::path&,
            ExistingOutputPolicy
        );
    };

    /** Application-specific conversion of evaluated DIPL values to output files. */
    class Adapter {
      public:
        virtual ~Adapter() = default;
        /** Select values, validate them for the target application, and register its files. */
        virtual void plan(const Environment& env, AdapterContext& context) const = 0;
    };

    /**
     * Plan, validate, and write adapter outputs below output_dir. Existing
     * destinations are rejected by default. ReplaceRegistered replaces only
     * registered regular files after all outputs are staged, and restores them
     * if publication fails. snapshot is an optional relative DIPH5 path below
     * the same directory. Returns written paths in registration order.
     */
    std::vector<std::filesystem::path> run_adapter(
        const Environment& env,
        const Adapter& adapter,
        const std::filesystem::path& output_dir,
        const std::filesystem::path& snapshot = {},
        ExistingOutputPolicy policy = ExistingOutputPolicy::Reject
    );

    /** Run without a snapshot while specifying an existing-output policy. */
    inline std::vector<std::filesystem::path> run_adapter(
        const Environment& env,
        const Adapter& adapter,
        const std::filesystem::path& output_dir,
        ExistingOutputPolicy policy
    ) {
        return run_adapter(env, adapter, output_dir, {}, policy);
    }

    /** Parse a DIPfile and run the adapter on its evaluated environment. */
    std::vector<std::filesystem::path> run_adapter_project(
        const std::filesystem::path& project,
        const Adapter& adapter,
        const std::filesystem::path& output_dir,
        const std::filesystem::path& snapshot = {},
        ExistingOutputPolicy policy = ExistingOutputPolicy::Reject
    );

    /** Run a project without saving a snapshot while specifying an existing-output policy. */
    inline std::vector<std::filesystem::path> run_adapter_project(
        const std::filesystem::path& project,
        const Adapter& adapter,
        const std::filesystem::path& output_dir,
        ExistingOutputPolicy policy
    ) {
        return run_adapter_project(project, adapter, output_dir, {}, policy);
    }

    /** Load a DIPH5 environment and run the adapter on its retained values and metadata. */
    std::vector<std::filesystem::path> run_adapter_snapshot(
        const std::filesystem::path& input,
        const Adapter& adapter,
        const std::filesystem::path& output_dir,
        const std::filesystem::path& snapshot = {},
        ExistingOutputPolicy policy = ExistingOutputPolicy::Reject
    );

    /** Run from a DIPH5 input without saving another snapshot while specifying a policy. */
    inline std::vector<std::filesystem::path> run_adapter_snapshot(
        const std::filesystem::path& input,
        const Adapter& adapter,
        const std::filesystem::path& output_dir,
        ExistingOutputPolicy policy
    ) {
        return run_adapter_snapshot(input, adapter, output_dir, {}, policy);
    }
} // namespace snt::dip

#endif // SNT_DIP_ADAPTER_H
