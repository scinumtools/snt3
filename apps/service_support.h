/** @file
 * @brief Application scratch-directory ownership and shared API synchronization.
 */
#ifndef SNT_APP_SERVICE_SUPPORT_H
#define SNT_APP_SERVICE_SUPPORT_H

#include <filesystem>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace snt::app {
    /** Serialize API evaluations that share the process-wide PUQ registry. */
    inline std::mutex api_mutex;

    /** Own a unique scratch directory and remove its contents on destruction. */
    class TemporaryDirectory {
      public:
        std::filesystem::path path; ///< Owned directory; callers may create temporary files below it.
        /** Create a directory below parent, throwing if no unique name can be created. */
        explicit TemporaryDirectory(const std::filesystem::path& parent = std::filesystem::temp_directory_path()) {
            std::random_device random;
            for (int attempt = 0; attempt < 32; ++attempt) {
                auto candidate = parent / ("snt-service-" + std::to_string(random()) + "-" + std::to_string(random()));
                if (std::filesystem::create_directory(candidate)) {
                    path = std::move(candidate);
                    return;
                }
            }
            throw std::runtime_error("Cannot create SNT service temporary directory.");
        }
        /** Remove temporary files without throwing during stack unwinding. */
        ~TemporaryDirectory() {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
        /** Directory ownership cannot be copied. */
        TemporaryDirectory(const TemporaryDirectory&) = delete;
        /** Directory ownership cannot be reassigned. */
        TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
    };
} // namespace snt::app
#endif
