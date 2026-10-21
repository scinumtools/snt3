#ifndef SNT_VIEW_BROWSER_VIEW_H
#define SNT_VIEW_BROWSER_VIEW_H

#include "viewer_model.h"

namespace snt::view {

    /** Return a display label for the browser object kind. */
    const char* object_kind_name(const ObjectInfo& object);
    /** Draw the searchable object tree and update model selection. */
    void draw_browser(ViewerModel& model);

} // namespace snt::view

#endif
