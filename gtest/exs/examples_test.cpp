#include <array>
#include <cstdio>
#include <gtest/gtest.h>
#include <memory>

std::string run_program(const std::string& cmd) {
  std::array<char, 128> buffer = {};
  std::string result;

  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    throw std::runtime_error("popen() failed!");

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  return result;
}

TEST(Examples, SolverUniquePtr) {
  std::string output = run_program("./SolverUniquePtr");
  EXPECT_EQ(output, "false\n");
}

TEST(Examples, SolverCustom) {
  std::string output = run_program("./SolverCustom");
  EXPECT_EQ(output, "true\n");
}

TEST(Examples, SolverDefault) {
  std::string output = run_program("./SolverDefault");
  EXPECT_EQ(output, "797.5\n");
}

TEST(Examples, SolverArray) {
  std::string output = run_program("./SolverArray");
  EXPECT_EQ(output, "{5}\n{5 7 9}\n");
}

TEST(Examples, SolverSettings) {
  std::string output = run_program("./SolverSettings");
  EXPECT_EQ(output, "3\n");
}
