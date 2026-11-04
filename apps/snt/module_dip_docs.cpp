#include "main.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

namespace {

struct Doc {
    const char* key;
    const char* title;
    const char* source;
    const unsigned char* text;
};

struct Heading {
    const char* doc;
    const char* slug;
    const char* title;
    std::size_t start;
    std::size_t end;
};

#include "dip_docs_generated.inc"

const Doc* find_doc(std::string_view key) {
    if (key.substr(0, 7) == "syntax/")
        key.remove_prefix(7);
    for (const auto& doc : kDocs) {
        if (key == doc.key)
            return &doc;
    }
    return nullptr;
}

void print_text(const Doc& doc, const Heading* heading = nullptr) {
    const std::string_view content(reinterpret_cast<const char*>(doc.text));
    const auto start = heading ? heading->start : 0;
    const auto end = heading ? heading->end : content.size();
    std::cout.write(content.data() + start, static_cast<std::streamsize>(end - start));
    if (end == start || content[end - 1] != '\n')
        std::cout << '\n';
}

void print_chapters() {
    std::size_t width = std::string_view("Topic").size();
    for (const auto& doc : kDocs)
        width = std::max(width, std::string_view(doc.key).size());
    const auto row = [width](std::string_view topic, std::string_view title) {
        std::cout << topic << std::string(width - topic.size() + 2, ' ') << title << '\n';
    };
    constexpr std::string_view title_header = "Title";
    row("Topic", title_header);
    row(std::string(width, '-'), std::string(title_header.size(), '-'));
    for (const auto& doc : kDocs)
        row(doc.key, doc.title);
}

void print_headings(const Doc& doc) {
    std::size_t width = std::string_view("Topic").size();
    for (const auto& heading : kHeadings) {
        if (std::string_view(heading.doc) == doc.key)
            width = std::max(width, std::string_view(doc.key).size() + 1 + std::string_view(heading.slug).size());
    }
    const auto row = [width](std::string_view topic, std::string_view title) {
        std::cout << topic << std::string(width - topic.size() + 2, ' ') << title << '\n';
    };
    constexpr std::string_view heading_header = "Heading";
    row("Topic", heading_header);
    row(std::string(width, '-'), std::string(heading_header.size(), '-'));
    for (const auto& heading : kHeadings) {
        if (std::string_view(heading.doc) == doc.key) {
            const std::string topic = std::string(doc.key) + '#' + heading.slug;
            row(topic, heading.title);
        }
    }
}

} // namespace

int module_dip_docs(int argc, char* argv[]) {
    if (argc == 1 || (argc == 2 && std::string_view(argv[1]) == "--list")) {
        print_chapters();
        return 0;
    }
    if (argc == 2 && (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h")) {
        std::cout << "Usage: snt dip docs [--list | CHAPTER [--list] | CHAPTER#HEADING]\n"
                     "Print embedded DIPL Markdown. Use --list after a chapter to list its headings.\n"
                     "Examples: snt dip docs expressions; snt dip docs expressions#templates\n";
        return 0;
    }
    if (argc == 3 && std::string_view(argv[2]) == "--list") {
        if (const auto* doc = find_doc(argv[1])) {
            print_headings(*doc);
            return 0;
        }
    } else if (argc == 2) {
        const std::string_view request(argv[1]);
        const auto separator = request.find('#');
        const auto key = request.substr(0, separator);
        if (const auto* doc = find_doc(key)) {
            if (separator == std::string_view::npos) {
                print_text(*doc);
                return 0;
            }
            const auto feature = request.substr(separator + 1);
            for (const auto& heading : kHeadings) {
                if (std::string_view(heading.doc) == doc->key && feature == heading.slug) {
                    print_text(*doc, &heading);
                    return 0;
                }
            }
            std::cerr << "Unknown heading in " << doc->key << ": " << feature
                      << ". Use 'snt dip docs " << doc->key << " --list'.\n";
            return 2;
        }
        if (separator == std::string_view::npos) {
            const Heading* match = nullptr;
            for (const auto& heading : kHeadings) {
                if (key != heading.slug)
                    continue;
                if (match) {
                    std::cerr << "Ambiguous DIPL heading: " << key
                              << ". Use CHAPTER#HEADING; see 'snt dip docs --list'.\n";
                    return 2;
                }
                match = &heading;
            }
            if (match) {
                print_text(*find_doc(match->doc), match);
                return 0;
            }
        }
    }
    std::cerr << "Unknown DIPL documentation topic. Use 'snt dip docs --list'.\n";
    return 2;
}
