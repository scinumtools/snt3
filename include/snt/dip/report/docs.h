#ifndef SNT_DIP_REPORT_DOCS_H
#define SNT_DIP_REPORT_DOCS_H

#include <filesystem>
#include <string>
#include <vector>

namespace snt::dip::report {
    struct DocsOptions {
        std::filesystem::path root;
        std::filesystem::path output_dir;
        bool html = true;
        bool pdf = false;
        std::string pdf_engine = "xelatex";
        bool pdf_full_reference = false; ///< Include every setting and schema member in the PDF.
        std::filesystem::path cheatsheet_pdf; ///< Optional prebuilt DIPL/PUEL appendix for PDF output.
        std::string title;
    };

    struct DocsResult {
        size_t projects = 0;
        size_t pages = 0;
        std::vector<std::string> warnings;
    };

    /** Generate a directory-oriented reference for all DIPfile projects under root. */
    DocsResult generate_docs(const DocsOptions& options);
}

#endif
