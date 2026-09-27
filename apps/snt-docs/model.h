#ifndef SNT_DOCS_MODEL_H
#define SNT_DOCS_MODEL_H

#include <snt/dip/environment.h>
#include <string>
#include <vector>

namespace snt::docs {

struct Publication {
    std::string authors, title, journal, year, volume, issue, pages, doi, url;
    bool empty() const;
};

struct Origin {
    std::string name, path, code;
    size_t line = 0;
};

struct Parameter {
    std::string path, value, units, description;
    std::vector<std::string> applied_schemas;
    std::string contributing_schema;
    bool overridden = false;
    Origin declaration, replacement;
    Publication publication;
};

struct Schema {
    std::string name, description;
    Origin origin;
    Publication publication;
};

struct Structure {
    std::string path, kind;
    std::vector<std::string> schemas;
};

struct Source {
    std::string name, path, parent, hash_algorithm, hash;
    size_t parent_line = 0;
};

struct Unit {
    std::string name, definition;
};

struct Function {
    std::string name, kind;
};

struct Document {
    std::string input_label;
    std::string introduction_tex;
    bool loaded_snapshot = false;
    std::vector<Parameter> parameters;
    std::vector<Structure> structure;
    std::vector<Schema> schemas;
    std::vector<Source> sources;
    std::vector<Unit> units;
    std::vector<Function> functions;
};

Document build_document(const dip::Environment& env, std::string input_label, std::string introduction_tex,
                        bool loaded_snapshot = false);

} // namespace snt::docs

#endif
