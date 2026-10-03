#ifndef SNT_VIEW_INSPECTOR_VIEW_H
#define SNT_VIEW_INSPECTOR_VIEW_H

#include "viewer_model.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace snt::view {

class SourceView;

struct InspectorCache {
    std::string path;
    std::size_t revision = 0;
    bool valid = false;
    std::vector<std::pair<const char*, std::string>> fields;
    std::vector<std::string> reads;
    std::vector<std::string> readers;
    std::vector<std::string> schema_values;
    std::vector<SourceTarget> source_targets;
    std::string error;
};

void draw_inspector(ViewerModel& model, InspectorCache& cache, SourceView& source,
                    bool& select_source_tab, bool& scroll_to_target, std::string& source_error);
void draw_source_view(const ViewerModel& model, const SourceView& source,
                      bool& scroll_to_target, const std::string& error);

} // namespace snt::view

#endif
