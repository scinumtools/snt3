/** @file
 * @brief Native MCP worker supervision and cancellation.
 */
#ifndef SNT_MCP_EXECUTION_H
#define SNT_MCP_EXECUTION_H
#include "../service_support.h"
#include "storage.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

namespace snt::mcp {
    /** Own session storage and supervise one native C++ tool worker at a time. */
    class Execution {
        app::TemporaryDirectory directory_; ///< Session directory removed with this supervisor.
        Json projects_;                     ///< Serializable mapping of environment names to input paths.
        unsigned timeout_ms_;               ///< Maximum wall-clock execution time for a tool.
        /** Cancellation state shared by the transport and worker supervisor. */
        struct Active {
            Json id;                            ///< JSON-RPC identifier of the running request.
            std::atomic<bool> cancelled{false}; ///< Set by the transport when cancellation is requested.
        };
        mutable std::mutex mutex_;       ///< Protects access to the active request pointer.
        std::shared_ptr<Active> active_; ///< Active request, or null when a worker may start.
      public:
        /** Capture configured inputs and the per-tool wall-clock deadline in milliseconds. */
        Execution(Json projects, unsigned timeout_ms) : projects_(std::move(projects)), timeout_ms_(timeout_ms) {}
        /** Create a read-only view of the published session artifacts. */
        Storage storage() const { return Storage(directory_.path); }
        /** Execute a tool in the native binary; return null on cancellation or a structured result otherwise. */
        Json run(
            const Json& id, const std::string& tool, const Json& arguments, const std::function<bool()>& disconnected
        );
        /** Request cancellation only when the identifier matches the active tool call. */
        void cancel(const Json& id);
        /** Report whether a tool is active; additional tool calls receive a retryable busy error. */
        bool busy() const;
    };
    /** Run one internal file-based request using native SNT APIs and write response.json beside it. */
    int worker_main(const std::filesystem::path& request);
} // namespace snt::mcp
#endif
