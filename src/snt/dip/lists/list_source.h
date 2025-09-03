#ifndef DIP_LIST_SOURCE_H
#define DIP_LIST_SOURCE_H

#include "../settings.h"
#include "list_node.h"

#include <string>

namespace snt::dip {

  class SourceList; // EnvSource needs a forward declaration

  struct EnvSource {
    std::string name; // source key
    std::string path; // source path
    std::string code; // source code
    Source parent;    // parent source
    NodeList nodes;   // parsed nodes
                      // std::shared_ptr<SourceList> sources;
  };

  class SourceList {
  private:
    std::map<std::string, EnvSource> sources;

  public:
    SourceList();
    void append(const std::string& name, const std::string& path, const std::string& code,
                const Source& parent);
    void append(const std::string& name, const EnvSource& src);
    EnvSource& at(const std::string& name);
    const EnvSource& at(const std::string& name) const;
  };

} // namespace snt::dip

#endif // DIP_LIST_SOURCE_H
