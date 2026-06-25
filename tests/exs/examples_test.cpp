#include "pch_tests.h"

#include <array>
#include <cstdio>
#include <filesystem>
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
    std::filesystem::path exe = "bin";
    exe /= "ExampleSolverUniquePtr";
#ifdef _WIN32
    exe += ".exe";
#endif
    std::string output = run_program(exe.string());
    EXPECT_EQ(output, "false\n");
}

TEST(Examples, SolverCustom) {
    std::filesystem::path exe = "bin";
    exe /= "ExampleSolverCustom";
#ifdef _WIN32
    exe += ".exe";
#endif
    std::string output = run_program(exe.string());
    EXPECT_EQ(output, "true\n");
}

TEST(Examples, SolverDefault) {
    std::filesystem::path exe = "bin";
    exe /= "ExampleSolverDefault";
#ifdef _WIN32
    exe += ".exe";
#endif
    std::string output = run_program(exe.string());
    EXPECT_EQ(output, "797.5\n");
}

TEST(Examples, SolverArray) {
    std::filesystem::path exe = "bin";
    exe /= "ExampleSolverArray";
#ifdef _WIN32
    exe += ".exe";
#endif
    std::string output = run_program(exe.string());
    EXPECT_EQ(output, "{5}\n{5 7 9}\n");
}

TEST(Examples, SolverSettings) {
    std::filesystem::path exe = "bin";
    exe /= "ExampleSolverSettings";
#ifdef _WIN32
    exe += ".exe";
#endif
    std::string output = run_program(exe.string());
    EXPECT_EQ(output, "3\n");
}
