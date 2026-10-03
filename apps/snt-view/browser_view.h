#ifndef SNT_VIEW_BROWSER_VIEW_H
#define SNT_VIEW_BROWSER_VIEW_H

#include "viewer_model.h"

namespace snt::view {

const char* object_kind_name(const ObjectInfo& object);
void draw_browser(ViewerModel& model);

} // namespace snt::view

#endif
