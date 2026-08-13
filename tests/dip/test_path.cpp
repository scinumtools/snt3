#include "pch_tests.h"

#include <snt/dip/nodes/path.h>

using namespace snt;

TEST(Path, FullyQualifiedPaths) {

    dip::Path path("foo.bar[0].baz[pop]");

    EXPECT_EQ(path.name, "foo.bar[0].baz[pop]");

    try {
        path = dip::Path("foo#.[2d).skd");
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Path is not fully qualified: foo#.[2d).skd");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
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
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Relative path wants to access parents beyong root node: ....snap.crackle");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}
