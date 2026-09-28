#ifndef SNT_REPORT_WRITER_H
#define SNT_REPORT_WRITER_H

#include "model.h"
#include <string>

namespace snt::dip::report {
std::string render_tex(const Document& document);
}

#endif
