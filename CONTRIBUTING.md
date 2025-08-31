# Contributing to scinumtools3

Thank you for considering contributing to **scinumtools3**! We welcome all contributions, including bug reports, feature requests, documentation improvements, and code changes.

---

## How to Contribute

1. **Fork the repository** from GitHub:  
   [https://github.com/vrtulka23/scinumtools3](https://github.com/vrtulka23/scinumtools3)

2. **Clone your fork locally**:
   ```bash
   git clone https://github.com/your-username/scinumtools3.git
   cd scinumtools3
   ```

3. **Create a new branch** for your work:

   ```bash
   git checkout -b feature/my-new-feature
   ```

4. **Make your changes**:

   * Follow the coding style defined in `.clang-format`.
   * Use modern C++ (C++17 or newer).
   * Write Doxygen-style comments for public APIs.
   * Add unit tests for new features or bug fixes (see `gtest/`).

5. **Build and run tests**:

   ```bash
   mkdir build && cd build
   cmake ..
   make
   ctest
   ```

6. **Commit and push your changes**:

   ```bash
   git add .
   git commit -m "Describe your change"
   git push origin feature/my-new-feature
   ```

7. **Open a Pull Request**:

   * Go to the original repository on GitHub.
   * Submit a PR with a clear description of your changes.
   * Link related issues if any.

---

## Coding Guidelines

* Use descriptive variable and function names.
* Keep functions small and focused.
* Apply `.clang-format` for consistent style.
* Document all public interfaces with Doxygen comments, for example:

  ```cpp
  /// \brief Short description
  /// \param name Description of the parameter
  /// \return Description of the return value
  ```

---

## Reporting Issues

If you find a bug or have a feature request:

1. Check the issue tracker to see if it is already reported:
   [https://github.com/vrtulka23/scinumtools3/issues](https://github.com/vrtulka23/scinumtools3/issues)

2. If not, open a new issue and include:

   * Steps to reproduce the problem
   * Expected behavior
   * Actual behavior
   * System details (OS, compiler, version)

---

## Communication

For questions or discussions:

* Use GitHub Discussions (if enabled) or open an Issue.

---

Thank you for contributing to **scinumtools3**!
