#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_source.h>
#include <stdexcept>

namespace snt::dip {

    SourceList::SourceList() = default;

    void SourceList::append(
        const std::string& name, const std::filesystem::path& path, const std::string& code, const Source& parent
    ) {
        auto it = sources.find(name);
        if (it != sources.end())
            throw dip::EnvironmentException(
                "Duplicate source",
                "A source named `" + name + "` already exists in the source list.",
                "Choose a different source name.",
                __FILE__,
                __LINE__
            );
        sources.insert({name, {name, path.string(), code, parent}});
    }

    void SourceList::append(const std::string& name, const EnvSource& src) {
        auto it = sources.find(name);
        if (it != sources.end())
            throw dip::EnvironmentException(
                "Duplicate source",
                "A source named `" + name + "` already exists in the source list.",
                "Choose a different source name.",
                __FILE__,
                __LINE__
            );
        sources.insert({name, src});
    }

    EnvSource& SourceList::at(const std::string& name) {
        auto it = sources.find(name);
        if (it == sources.end())
            throw dip::EnvironmentException(
                "Unknown source",
                "The source `" + name + "` was not found in the environment source list.",
                "Check whether the source name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

    const EnvSource& SourceList::at(const std::string& name) const {
        auto it = sources.find(name);
        if (it == sources.end())
            throw dip::EnvironmentException(
                "Unknown source",
                "The source `" + name + "` was not found in the environment source list.",
                "Check whether the source name is correct.",
                __FILE__,
                __LINE__
            );
        return it->second;
    }

} // namespace snt::dip
