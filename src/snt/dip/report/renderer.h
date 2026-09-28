#ifndef SNT_REPORT_RENDERER_H
#define SNT_REPORT_RENDERER_H

#include "model.h"
#include <snt/dip/report/report.h>
#include <string>

namespace snt::dip::report {
std::string render_document(const Document& document, ReportFormat format);
}

#endif
