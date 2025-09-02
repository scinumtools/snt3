#ifndef PUQ_BACKTRACE_H
#define PUQ_BACKTRACE_H

#include <cstdlib>    // For free
#include <cxxabi.h>   // For __cxa_demangle (optional)
#include <execinfo.h> // For backtrace and backtrace_symbols
#include <iostream>

void print_backtrace() {
  /*
    const int max_frames = 128;
    void* buffer[max_frames];  // Buffer to store backtrace pointers

    // Capture the backtrace (returns the number of frames captured)
    int num_frames = backtrace(buffer, max_frames);

    // Get symbols for the backtrace
    char** symbols = backtrace_symbols(buffer, num_frames);

    // Print the backtrace
    std::cout << "Backtrace (" << num_frames << " frames):\n";
    for (int i = 0; i < num_frames; ++i) {
        // Optionally demangle the function name
        char* demangled_name = symbols[i];
        char* left_paren = nullptr;
        char* plus_sign = nullptr;

        // Attempt to find the mangled name
        for (char* p = symbols[i]; *p; ++p) {
            if (*p == '(') left_paren = p;
            else if (*p == '+') plus_sign = p;
        }

        if (left_paren && plus_sign && left_paren < plus_sign) {
            *plus_sign = '\0';  // Null-terminate before the '+'
            int status;
            demangled_name = abi::__cxa_demangle(left_paren + 1, nullptr, nullptr, &status);
        }

        std::cout << "[" << i << "] " << symbols[i] << (demangled_name ? demangled_name : "") << "\n";

        // Free the demangled name if it was allocated
        if (demangled_name != symbols[i]) free(demangled_name);
    }

    // Free the symbols buffer
    free(symbols);
  */
  void* array[10];
  char** strings;
  int size, i;

  size = backtrace(array, 10);
  strings = backtrace_symbols(array, size);
  if (strings != nullptr) {

    printf("Obtained %d stack frames.\n", size);
    for (i = 0; i < size; i++)
      printf("%s\n", strings[i]);
  }

  free(strings);
}

#endif
