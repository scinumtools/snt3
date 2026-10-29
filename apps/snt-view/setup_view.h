#ifndef SNT_VIEW_SETUP_VIEW_H
#define SNT_VIEW_SETUP_VIEW_H

#include "viewer_model.h"

#include <map>
#include <string>

namespace snt::view {
    class NotificationCenter;

    struct SetupViewState {
        std::size_t revision = static_cast<std::size_t>(-1);
        std::map<std::string, std::string> answers;
        std::map<std::string, std::string> text;
        std::map<std::string, std::size_t> choices;
        std::string output_name = "setup-overrides.dip";
        std::string error_path;
        bool scroll_to_error = false;
    };

    /** Draw all guided setup inputs and commit them together. */
    void draw_setup_view(ViewerModel& model, SetupViewState& state, NotificationCenter& notifications);
} // namespace snt::view

#endif
