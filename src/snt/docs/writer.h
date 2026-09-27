#ifndef SNT_DOCS_WRITER_H
#define SNT_DOCS_WRITER_H

#include "model.h"
#include <string>

namespace snt::docs {
std::string render_tex(const Document& document);
}

#endif
