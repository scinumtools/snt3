/** @file
 * @brief Viewer object browser rendering.
 */
#ifndef SNT_VIEW_BROWSER_VIEW_H
#define SNT_VIEW_BROWSER_VIEW_H

#include "viewer_model.h"

#include <imgui.h>

namespace snt::view {

    /** Return a display label for the browser object kind. */
    const char* object_kind_name(const ObjectInfo& object);
    /** Draw the same semantic icon used for an entry in the browser tree. */
    void draw_object_icon(const ObjectInfo& object, ImVec2 position, float size);
    /** Draw the searchable object tree and update model selection. */
    void draw_browser(ViewerModel& model);

} // namespace snt::view

#endif
