#ifndef SNT_REPORT_MODEL_H
#define SNT_REPORT_MODEL_H

#include <snt/dip/environment.h>
#include <string>
#include <vector>

namespace snt::dip::report {

struct Publication {
    std::string authors, title, journal, year, volume, issue, pages, doi, url;
};

struct Origin {
    std::string name, path, code;
    size_t line = 0;
};

struct Parameter {
    std::string path, value, units, type, shape, description;
    std::vector<std::string> applied_schemas;
    std::string contributing_schema;
    bool overridden = false;
    Origin declaration, replacement;
    std::vector<Origin> modifications;
    Publication publication;
};

struct TableColumn {
    std::string name, type, units;
};

struct Table {
    std::string path;
    size_t rows = 0;
    std::vector<TableColumn> columns;
};

struct Schema {
    std::string name, description;
    Origin origin;
    Publication publication;
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
    std::string title, author, date, version;
    bool loaded_snapshot = false;
    std::vector<Parameter> parameters;
    std::vector<Table> tables;
    std::vector<Schema> schemas;
    std::vector<Source> sources;
    std::vector<Unit> units;
    std::vector<Function> functions;
};

Document build_document(const dip::Environment& env, std::string input_label, std::string introduction_tex,
                        std::string title, std::string author, std::string date, std::string version,
                        const std::filesystem::path& source_root, bool loaded_snapshot = false);

} // namespace snt::dip::report

#endif
