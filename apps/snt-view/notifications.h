/** @file
 * @brief Shared viewer notification center.
 */
#ifndef SNT_VIEW_NOTIFICATIONS_H
#define SNT_VIEW_NOTIFICATIONS_H

#include <string>
#include <unordered_map>
#include <vector>

namespace snt::view {

    enum class NoticeLevel { Info, Warning, Error };

    class NotificationCenter {
      public:
        void notify(std::string message, NoticeLevel level = NoticeLevel::Info);
        void observe(const std::string& key, const std::string& message, NoticeLevel level);
        void draw();

      private:
        struct Notice {
            unsigned long long id;
            std::string message;
            NoticeLevel level;
            double expires_at;
            int created_frame;
        };
        std::vector<Notice> notices_;
        std::unordered_map<std::string, std::string> observed_;
        unsigned long long next_id_ = 1;
    };

} // namespace snt::view

#endif
