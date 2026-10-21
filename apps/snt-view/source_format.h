/** @file
 * @brief Source highlighting modes used by the viewer.
 */
#ifndef SNT_VIEW_SOURCE_FORMAT_H
#define SNT_VIEW_SOURCE_FORMAT_H

namespace snt::view {

    /** Select highlighting for a source buffer. */
    enum class SourceFormat {
        DIPL,  ///< Highlight DIPL syntax throughout the buffer.
        Plain, ///< Display text without syntax highlighting.
        Table  ///< Highlight the DIPL header before the table separator.
    };

} // namespace snt::view

#endif
