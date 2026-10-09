#ifndef SNT_DIP_ARTIFACT_INPUT_H
#define SNT_DIP_ARTIFACT_INPUT_H

#include <snt/dip/dip.h>

#include <filesystem>

namespace snt::dip::detail {

/** Register a parseable project or DIPL input on a fresh parser. */
void register_parse_input(DIP& parser, const std::filesystem::path& path);

} // namespace snt::dip::detail

#endif
