/** @file
 * @brief Source buffers and syntax highlighting.
 */
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

    /** A source buffer with DIPL highlighting or plain text. */
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
        /** Open packaged text for a regular document while preserving its recorded line target. */
        bool open_document(
            const std::filesystem::path& file,
            std::size_t line,
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
        /** Return the display format associated with this source. */
        SourceFormat format() const { return format_; }
        /** Replace the current buffer text and refresh syntax highlighting. */
        void set_text(std::string text);
        /** Test whether the buffer differs from its last loaded or saved version. */
        bool dirty() const { return text_ != saved_text_; }
        /** Return whether the source editor is active. */
        bool editing() const { return editing_; }
        /** Switch between the source renderer and editable text control. */
        void set_editing(bool editing) { editing_ = editing; }
        /** Mark the current text as saved. */
        void mark_saved() { saved_text_ = text_; }
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
        std::string saved_text_;              ///< Text snapshot used to track unsaved edits.
        SourceFormat format_ = SourceFormat::DIPL;
        bool editing_ = false;
        std::vector<HighlightedLine> lines_; ///< Tokenized lines for rendering.
    };

} // namespace snt::view

#endif
