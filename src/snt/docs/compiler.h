#ifndef SNT_DOCS_COMPILER_H
#define SNT_DOCS_COMPILER_H

#include <filesystem>
#include <string>

namespace snt::docs {
void write_file(const std::filesystem::path& path, const std::string& contents);
void compile_pdf(const std::string& contents, const std::filesystem::path& output, const std::string& compiler);
}

#endif
