#include <gtest/gtest.h>
#include <snt/dip/tools/formatter.h>

using snt::dip::tools::format;
using snt::dip::tools::FormatOptions;

TEST(DipFormatter, GroupsSiblingNodesAndKeepsAnnotationsAttached) {
    const std::string input = "run\n"
                              "  speed int=2\n"
                              "    !options [1,2, 3]\n"
                              "    !labels [\"slow\",\"fast\",\"max\"]\n"
                              "  # description of mode\n"
                              "  mode str=\"fast\"\n"
                              "    ?descr \"chosen mode\"\n"
                              "next int=4\n";
    const std::string expected = "run\n"
                                 "  speed int = 2\n"
                                 "    !options [1, 2, 3]\n"
                                 "    !labels [\"slow\", \"fast\", \"max\"]\n"
                                 "\n"
                                 "  # description of mode\n"
                                 "  mode str = \"fast\"\n"
                                 "    ?descr \"chosen mode\"\n"
                                 "\n"
                                 "next int = 4\n";
    EXPECT_EQ(format(input), expected);
    EXPECT_EQ(format(expected), expected);
}

TEST(DipFormatter, PreservesQuotedPunctuationAndMultilineContents) {
    const std::string input = "text str=\"a,b=x#y\"\n"
                              "body str = \"\"\"\n"
                              " untouched,  x =  y  \n"
                              "\"\"\"\n"
                              "items str = [\"x,y\",\"z\"]\n";
    const std::string expected = "text str = \"a,b=x#y\"\n"
                                 "\n"
                                 "body str = \"\"\"\n"
                                 " untouched,  x =  y  \n"
                                 "\"\"\"\n"
                                 "\n"
                                 "items str = [\"x,y\", \"z\"]\n";
    EXPECT_EQ(format(input), expected);
}

TEST(DipFormatter, RespectsZeroSpacingAndFinalNewline) {
    FormatOptions options;
    options.blank_lines_between_nodes = 0;
    options.final_newline = false;
    EXPECT_EQ(format("a int=1\n\nb int=2\n", options), "a int = 1\nb int = 2");
}

TEST(DipFormatter, FormatsSchemaSeparatorAndInlineTripleString) {
    const std::string input = "settings:profile\nmessage str=\"\"\"a,b=x\"\"\"\nnext int=2\n";
    const std::string expected = "settings : profile\n\nmessage str = \"\"\"a,b=x\"\"\"\n\nnext int = 2\n";
    EXPECT_EQ(format(input), expected);
}

TEST(DipFormatter, KeepsCommentsInMultilineOptionArrays) {
    const std::string input = "mode str=\"a\"\n"
                              "  !options [\n"
                              "    \"a\", # first\n"
                              "    \"b\"\n"
                              "  ]\n"
                              "next int=1\n";
    const std::string expected = "mode str = \"a\"\n"
                                 "  !options [\n"
                                 "    \"a\", # first\n"
                                 "    \"b\"\n"
                                 "  ]\n"
                                 "\n"
                                 "next int = 1\n";
    EXPECT_EQ(format(input), expected);
}

TEST(DipFormatter, KeepsCRLFLineEndings) {
    const std::string input = "a int=1\r\nb int=2\r\n";
    EXPECT_EQ(format(input), "a int = 1\r\n\r\nb int = 2\r\n");
}

TEST(DipFormatter, LeavesPathAndDimensionCommasAlone) {
    EXPECT_EQ(format("map[a,b] int=1\nshape float[2,3]=[1,2]\n"), "map[a,b] int = 1\n\nshape float[2,3] = [1, 2]\n");
}
