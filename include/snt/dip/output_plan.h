#ifndef SNT_DIP_OUTPUT_PLAN_H
#define SNT_DIP_OUTPUT_PLAN_H

#include <snt/dip/nodes/node_value.h>
#include <string>
#include <vector>

namespace snt::dip {
    class Environment;

    /** One adapter-owned mapping resolved against an evaluated environment. */
    struct OutputMapping {
        std::string id;
        std::string target;
        std::string key;
        std::string source_path; ///< Empty for a value calculated by the adapter.
        std::string rule;        ///< Stable, human-readable rule or callback name.
        std::string origin;      ///< Adapter/profile declaration identity.
        std::vector<std::string> replacements; ///< Previous declaration origins.
        std::vector<std::string> dependencies;
        bool active = true;
        ValueNode::PointerType value; ///< Typed copy, outside Environment::nodes.
    };

    /** Validated, ordered output mappings. Tags are only used by adapters to discover nodes. */
    class OutputPlan {
      public:
        void add_node(const Environment& env, std::string id, std::string target, std::string key,
                      std::string source_path, bool active = true, std::string rule = {},
                      std::string origin = {}, std::vector<std::string> dependencies = {});
        void replace_node(const Environment& env, std::string id, std::string target, std::string key,
                          std::string source_path, bool active = true, std::string rule = {},
                          std::string origin = {}, std::vector<std::string> dependencies = {});
        void add_value(std::string id, std::string target, std::string key, ValueNode::PointerType value,
                       bool active = true, std::string rule = {}, std::string origin = {},
                       std::vector<std::string> dependencies = {});
        void replace_value(std::string id, std::string target, std::string key, ValueNode::PointerType value,
                           bool active = true, std::string rule = {}, std::string origin = {},
                           std::vector<std::string> dependencies = {});

        /** Check active native-key collisions after all deliberate replacements. */
        void validate() const;
        const std::vector<OutputMapping>& mappings() const { return mappings_; }
        std::vector<OutputMapping> select(const std::string& target, bool active_only = false) const;

        /** Restore a validated mapping from a versioned DIPH5 snapshot. */
        void restore(OutputMapping mapping);

      private:
        std::vector<OutputMapping> mappings_;
        void insert(OutputMapping mapping, bool replace);
    };
} // namespace snt::dip

#endif
