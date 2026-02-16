#include "pch_tests.h"

#include <array>
#include <cstdio>
#include <memory>

std::string run_program(const std::string& cmd) {
  std::array<char, 128> buffer = {};
  std::string result;

  std::unique_ptr<FILE, int (*)(FILE*)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    throw std::runtime_error("popen() failed!");

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  return result;
}

TEST(Examples, SolverUniquePtr) {
  std::string output = run_program("./bin/ExampleSolverUniquePtr");
  EXPECT_EQ(output, "false\n");
}

TEST(Examples, SolverCustom) {
  std::string output = run_program("./bin/ExampleSolverCustom");
  EXPECT_EQ(output, "true\n");
}

TEST(Examples, SolverDefault) {
  std::string output = run_program("./bin/ExampleSolverDefault");
  EXPECT_EQ(output, "797.5\n");
}

TEST(Examples, SolverArray) {
  std::string output = run_program("./bin/ExampleSolverArray");
  EXPECT_EQ(output, "{5}\n{5 7 9}\n");
}

TEST(Examples, SolverSettings) {
  std::string output = run_program("./bin/ExampleSolverSettings");
  EXPECT_EQ(output, "3\n");
}
