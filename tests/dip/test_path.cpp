#include "pch_tests.h"

#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/path.h>

using namespace snt;

TEST(Path, FullyQualifiedPaths) {

    dip::Path path("foo.bar[0].baz[pop]");

    EXPECT_EQ(path.name, "foo.bar[0].baz[pop]");

    try {
        path = dip::Path("foo#.[2d).skd");
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Path is not fully qualified");
        EXPECT_EQ(
            e.info().details,
            "The path contains unexpected characters after the complete path expression: `foo#.[2d).skd`."
        );
        EXPECT_EQ(
            e.info().suggestion,
            "Remove the unexpected characters and ensure that path nodes names contain only letters, digits, "
            "underscores, and hyphens."
        );
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}

TEST(Path, ResolveRelativePaths) {

    dip::Path path("foo.bar[0].baz[pop]");

    dip::Path resolved = path.resolve(".");
    EXPECT_EQ(resolved.name, path.name);

    resolved = path.resolve(".snap.crackle");
    EXPECT_EQ(resolved.name, "foo.bar[0].snap.crackle");

    resolved = path.resolve("..snap.crackle");
    EXPECT_EQ(resolved.name, "foo.snap.crackle");

    resolved = path.resolve("...snap.crackle");
    EXPECT_EQ(resolved.name, "snap.crackle");

    try {
        resolved = path.resolve("....snap.crackle");
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Relative path exceeds root");
        EXPECT_EQ(
            e.info().details, "The relative path attempts to access a parent beyond the root node: `....snap.crackle`."
        );
        EXPECT_EQ(
            e.info().suggestion,
            "Reduce the number of parent references `.` so that the path does not go beyond the root node."
        );
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}
