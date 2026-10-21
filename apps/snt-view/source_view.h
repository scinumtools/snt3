#ifndef SNT_VIEW_SOURCE_VIEW_H
#define SNT_VIEW_SOURCE_VIEW_H

#include "source_format.h"

#include <cstddef>
#include <dipl_highlighter.h>
#include <filesystem>
#include <string>
#include <vector>

namespace snt::view {

    using SyntaxKind = dipl::highlight::SyntaxKind;           ///< Token categories shared with the DIPL highlighter.
    using HighlightedLine = dipl::highlight::HighlightedLine; ///< Line text and highlighted token spans.

    /** A read-only file buffer with DIPL highlighting or plain text. */
    class SourceView {
      public:
        /** Read a file and select its one-based line. Return false and set error on read or content failure. */
        bool open(
            const std::filesystem::path& file,
            std::size_t line,
            std::string& error,
            SourceFormat format = SourceFormat::DIPL
        );
        /** Display supplied block text, retaining its original file and source line for provenance. */
        bool open_text(
            const std::filesystem::path& file,
            std::size_t source_line,
            std::string content,
            SourceFormat format,
            std::string& error
        );
        /** Return the file associated with this buffer. */
        const std::filesystem::path& file() const { return file_; }
        /** Return the one-based line to highlight within the displayed buffer. */
        std::size_t target_line() const { return target_line_; }
        /** Return the recorded one-based line in the original source file. */
        std::size_t source_line() const { return source_line_; }
        /** Return the complete loaded text. */
        const std::string& text() const { return text_; }
        /** Return lines tokenized for the selected display format. */
        const std::vector<HighlightedLine>& lines() const { return lines_; }

      private:
        /** Validate text, tokenize it, and replace the buffer; report stale locations through error. */
        bool set_content(
            const std::filesystem::path& file,
            std::size_t source_line,
            std::size_t target_line,
            std::string content,
            SourceFormat format,
            std::string& error
        );
        std::filesystem::path file_;         ///< Original source filename.
        std::size_t target_line_ = 0;        ///< One-based target in the displayed buffer; zero when unset.
        std::size_t source_line_ = 0;        ///< One-based location recorded in the original file.
        std::string text_;                   ///< Complete displayed text.
        std::vector<HighlightedLine> lines_; ///< Tokenized lines for rendering.
    };

} // namespace snt::view

#endif
