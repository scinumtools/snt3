#include "pch_tests.h"
#include <snt/dip/dip.h>

#include <filesystem>
#include <fstream>

using namespace snt;

TEST(DIP, AddString) {

  dip::DIP d;
  std::string code = "foo str = \"bar\"\ncount int = 3\nnew bool = true";
  d.add_string(code);
}

TEST(DIP, Addfile) {

  // create temporary file
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path temp_filename = temp_dir / "test_file.dip";
  {
    std::ofstream temp_file(temp_filename);
    ASSERT_TRUE(temp_file.is_open()) << "Failed to create temp file.";
    temp_file << "foo int = 3\nbar bool = false";
  }

  dip::DIP d;
  d.add_file(temp_filename);

  // remove_temporary file
  std::filesystem::remove(temp_filename);
}
