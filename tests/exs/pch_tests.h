#ifndef EXS_PCH_TESTS_H
#define EXS_PCH_TESTS_H

#include <gtest/gtest.h>

// Windows specific settings
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#endif // EXS_PCH_TESTS_H
