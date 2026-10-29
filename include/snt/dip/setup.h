#ifndef SNT_DIP_SETUP_H
#define SNT_DIP_SETUP_H

#include <filesystem>
#include <map>
#include <snt/dip/dip.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace snt::dip {

    /** A guided setup answer failed validation; path identifies the form field. */
    class SetupInputError : public std::runtime_error {
        std::string path_;

      public:
        SetupInputError(std::string path, std::string message)
            : std::runtime_error(std::move(message)), path_(std::move(path)) {}
        const std::string& path() const noexcept { return path_; }
    };

    struct SetupOption {
        std::string value;
        std::string label;
    };

    /** A user-editable value discovered from a DIPL `!setup` annotation. */
    struct SetupQuestion {
        std::string path;
        std::string description;
        NodeDtype type = NodeDtype::None;
        bool array = false;
        bool has_value = false;
        std::string current_value;
        std::vector<SetupOption> options;
    };

    /** Collect setup inputs, construct overrides, and validate them as one project. */
    class SetupSession {
      private:
        std::filesystem::path project_file_;
        std::vector<SetupQuestion> questions_;
        std::vector<std::pair<std::filesystem::path, std::size_t>> registered_overrides_;
        std::map<std::string, std::string> answers_;

        const SetupQuestion& question(const std::string& path) const;
        std::string answer_text(bool allow_incomplete) const;
        Environment parse_answers(bool allow_incomplete) const;

      public:
        explicit SetupSession(const std::filesystem::path& project_file);

        const std::vector<SetupQuestion>& questions() const { return questions_; }
        /** Number of guided setup override files registered in the project. */
        std::size_t registered_override_count() const { return registered_overrides_.size(); }

        /** Set a string as plain text, or other scalar/array values as DIPL expressions. */
        void answer(const std::string& path, const std::string& input);
        /** Select one of the DIPL `!options` values by its zero-based index. */
        void choose(const std::string& path, std::size_t option_index);
        /** Stage a complete form and validate the combined answers once. */
        void answer_batch(const std::map<std::string, std::string>& answers,
                          const std::map<std::string, std::size_t>& choices);
        /** The generated unwrapped DIPL override body. */
        std::string override_text() const;
        /** Parse the project with all answers and run its ordinary validation. */
        Environment preview() const;
        /** Validate and write the override body, refusing to replace an existing file. */
        void save(const std::filesystem::path& output_file) const;
        /** Validate and save answers, then register the new override in the DIPfile. */
        void save_and_register(const std::filesystem::path& output_file) const;
        /** Unregister and delete guided setup override files without touching other overrides. */
        void remove_registered_overrides() const;
    };

} // namespace snt::dip

#endif
