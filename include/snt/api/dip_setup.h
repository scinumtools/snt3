#ifndef SNT_API_DIP_SETUP_H
#define SNT_API_DIP_SETUP_H

#include <snt/dip/setup.h>

namespace snt::api {

    /** API entry point for the core DIPL guided setup session. */
    class DIPSetup : public dip::SetupSession {
      public:
        using dip::SetupSession::SetupSession;
    };

} // namespace snt::api

#endif
