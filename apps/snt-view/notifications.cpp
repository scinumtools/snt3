#include "notifications.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cstddef>
#include <unordered_set>
#include <utility>

namespace snt::view {

    void NotificationCenter::notify(std::string message, NoticeLevel level) {
        if (message.empty())
            return;
        const double now = ImGui::GetTime();
        notices_.erase(std::remove_if(notices_.begin(), notices_.end(), [now](const Notice& notice) {
            return notice.level == NoticeLevel::Info && now >= notice.expires_at;
        }), notices_.end());
        const int frame = ImGui::GetFrameCount();
        const auto existing = std::find_if(notices_.begin(), notices_.end(), [&](const Notice& notice) {
            return notice.level == level && notice.message == message && notice.created_frame == frame;
        });
        const double expiry = level == NoticeLevel::Info ? now + 4.0 : 0.0;
        if (existing != notices_.end())
            return;
        notices_.push_back({next_id_++, std::move(message), level, expiry, frame});
    }

    void NotificationCenter::observe(const std::string& key, const std::string& message, NoticeLevel level) {
        auto& previous = observed_[key];
        if (message.empty()) {
            previous.clear();
            return;
        }
        if (previous != message) {
            previous = message;
            notify(message, level);
        }
    }

    void NotificationCenter::draw() {
        const double now = ImGui::GetTime();
        notices_.erase(std::remove_if(notices_.begin(), notices_.end(), [now](const Notice& notice) {
            return notice.level == NoticeLevel::Info && now >= notice.expires_at;
        }), notices_.end());
        if (notices_.empty())
            return;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 12.0f,
                                      viewport->WorkPos.y + 12.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, 0.0f), ImVec2(400.0f, viewport->WorkSize.y - 24.0f));
        constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                          ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;
        std::unordered_set<unsigned long long> dismissed;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Viewer notifications", nullptr, flags)) {
            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
            for (std::size_t i = 0; i < notices_.size(); ++i) {
                const auto& notice = notices_[i];
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
                const ImVec4 color = notice.level == NoticeLevel::Info
                                         ? ImVec4(0.24f, 0.48f, 0.38f, 1.0f)
                                         : notice.level == NoticeLevel::Warning
                                               ? ImVec4(0.72f, 0.48f, 0.12f, 1.0f)
                                               : ImVec4(0.70f, 0.25f, 0.24f, 1.0f);
                const ImVec4 background = notice.level == NoticeLevel::Info
                                              ? ImVec4(0.93f, 0.97f, 0.94f, 1.0f)
                                          : notice.level == NoticeLevel::Warning
                                              ? ImVec4(0.99f, 0.96f, 0.88f, 1.0f)
                                              : ImVec4(0.99f, 0.92f, 0.91f, 1.0f);
                const ImVec2 card_padding(12.0f, 9.0f);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.0f);
                const float card_width = ImGui::GetContentRegionAvail().x - 4.0f;
                const float text_width = card_width - card_padding.x * 2.0f - 2.0f;
                const ImVec2 text_size = ImGui::CalcTextSize(notice.message.c_str(), nullptr, false, text_width);
                const float card_height = card_padding.y * 2.0f + ImGui::GetFrameHeight() +
                                          ImGui::GetStyle().ItemSpacing.y + text_size.y + 2.0f;
                ImGui::PushID(static_cast<int>(notice.id));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, background);
                ImGui::PushStyleColor(ImGuiCol_Border, color);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 7.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, card_padding);
                ImGui::BeginChild("notice", ImVec2(card_width, card_height), ImGuiChildFlags_Borders);
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(notice.level == NoticeLevel::Info ? "Info" :
                                       notice.level == NoticeLevel::Warning ? "Warning" : "Error");
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - 20.0f);
                const bool close = ImGui::SmallButton("x");
                ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x);
                ImGui::TextWrapped("%s", notice.message.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndChild();
                ImGui::PopStyleVar(3);
                ImGui::PopStyleColor(2);
                ImGui::PopID();
                if (close)
                    dismissed.insert(notice.id);
                ImGui::Dummy(ImVec2(0.0f, i + 1 < notices_.size() ? 5.0f : 2.0f));
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
        if (!dismissed.empty())
            notices_.erase(std::remove_if(notices_.begin(), notices_.end(), [&](const Notice& notice) {
                return dismissed.count(notice.id) != 0;
            }), notices_.end());
    }

} // namespace snt::view
