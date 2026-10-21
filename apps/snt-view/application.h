/** @file
 * @brief Desktop viewer event-loop entry point.
 */
#ifndef SNT_VIEW_APPLICATION_H
#define SNT_VIEW_APPLICATION_H

namespace snt::view {
    class ViewerModel;
    /** Create the viewer window and run its event loop using the supplied model. */
    int run_application(ViewerModel& model);
} // namespace snt::view

#endif
