from pathlib import Path
import re

from conan import ConanFile
from conan.tools.cmake import (
    CMake,
    CMakeDeps,
    CMakeToolchain,
    cmake_layout,
)
from conan.tools.files import copy


class SciNumToolsConan(ConanFile):
    #
    # Metadata
    #
    name = "scinumtools3"

    license = "MIT"
    url = "https://github.com/vrtulka23/scinumtools3"
    homepage = url
    description = "Scientific Numerical Tools v3 (SNT)"
    topics = (
        "scientific",
        "numerical",
        "math",
        "scientific-computing",
        "cpp",
    )

    package_type = "library"

    #
    # Settings
    #
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": True,
    }

    #
    # Export the complete project
    #
    exports_sources = (
        "CMakeLists.txt",
        "src/*",
        "include/*",
        "bindings/*",
        "cmake/*",
        "examples/*",
        "tests/*",
        "apps/*",
        "pyproject.toml",
        "LICENSE",
        "README.md",
        "settings.env",
    )

    #
    # Read version from settings.env
    #
    def set_version(self):
        settings_file = (
            Path(__file__).parent / "settings.env"
        )

        text = settings_file.read_text(encoding="utf-8")

        m = re.search(r"^CODE_VERSION=(.+)$", text, re.MULTILINE)
        if not m:
            raise RuntimeError("CODE_VERSION not found in settings.env")

        self.version = m.group(1).strip()

    #
    # Conan layout
    #
    def layout(self):
        cmake_layout(self)

    #
    # Remove fPIC on Windows
    #
    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    #
    # Generate toolchain
    #
    def generate(self):
        tc = CMakeToolchain(self)

        #
        # Build only the C++ library
        #
        tc.variables["ENABLE_BINDING_PYTHON"] = False
        tc.variables["ENABLE_UNIT_TESTS"] = False
        tc.variables["RUN_UNIT_TESTS"] = False
        tc.variables["ENABLE_MAT"] = False
        tc.variables["ENABLE_EXEC_APPS_DMAP"] = False
        tc.variables["ENABLE_EXEC_EXAMPLES"] = False
        tc.variables["ENABLE_EXEC_BENCHMARKS"] = False

        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    #
    # Build
    #
    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder="./")
        cmake.build()

    #
    # Install using your existing install() rules
    #
    def package(self):
        cmake = CMake(self)
        cmake.install()

        root = Path(__file__).resolve().parents[2]

        license_file = root / "LICENSE"

        if license_file.exists():
            copy(
                self,
                "LICENSE",
                src=str(root),
                dst="licenses",
            )

    #
    # Information for consumers
    #
    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "snt")
        self.cpp_info.set_property("cmake_target_name", "snt::snt")

        self.cpp_info.libs = ["snt"]
