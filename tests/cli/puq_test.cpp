#include <gtest/gtest.h>
#include <string>
#include "snt/argparser.h"
#include "snt/puq/main.h"

using namespace snt;

TEST(puq, Solve) {

  std::vector<std::string> args = {"snt", "puq", "solve", "23*cm + 3*m"};
  int argc = (int)args.size();
  std::vector<char*> argv;
  argv.reserve(argc);
  for (auto& s : args) {
    argv.push_back(s.data());
  }

  cli::ArgParser argpar(argc, argv.data());
  
  testing::internal::CaptureStdout();
  puq_main(argpar);
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_NE(output.find("323*cm"), std::string::npos);

}

TEST(puq, Convert) {

  std::vector<std::string> args = {"snt", "puq", "convert", "ESU", "12*statA", "SI", "A", "I"};
  int argc = (int)args.size();
  std::vector<char*> argv;
  argv.reserve(argc);
  for (auto& s : args) {
    argv.push_back(s.data());
  }

  cli::ArgParser argpar(argc, argv.data());
  
  testing::internal::CaptureStdout();
  puq_main(argpar);
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_NE(output.find("4.00277e-9*A"), std::string::npos);

}
