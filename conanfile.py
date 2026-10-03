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
        "with_server": [True, False],
        "with_viewer": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": True,
        "with_server": True,
        "with_viewer": True,
        "hdf5/*:enable_cxx": False,
        "hdf5/*:hl": False,
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
        "docs/dipl/highlight/cpp/*",
        "external/briefpp/include/briefpp/*",
        "external/briefpp/include/briefpp/renderers/*",
        "external/briefpp/LICENSE",
        "external/cpp-httplib/httplib.h",
        "external/cpp-httplib/LICENSE",
        "external/glfw/*",
        "external/imgui/*",
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
    def requirements(self):
        self.requires("hdf5/[>=1.14.3 <2]", transitive_headers=True, transitive_libs=True)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    #
    # Generate toolchain
    #
    def generate(self):
        tc = CMakeToolchain(self)

        #
        # Build the libraries and the unified CLI, including its server and viewer.
        #
        tc.variables["ENABLE_BINDING_PYTHON"] = False
        tc.variables["ENABLE_BINDING_C"] = False
        tc.variables["ENABLE_UNIT_TESTS"] = False
        tc.variables["RUN_UNIT_TESTS"] = False
        tc.variables["ENABLE_EXEC_APPS"] = True
        tc.variables["ENABLE_EXEC_APPS_SNT"] = True
        tc.variables["ENABLE_SNT_SERVER"] = bool(self.options.with_server)
        tc.variables["ENABLE_SNT_VIEW"] = bool(self.options.with_viewer)
        tc.variables["ENABLE_MAT"] = False
        tc.variables["ENABLE_SNT_DMAP"] = False
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

        copy(self, "LICENSE", src=self.source_folder,
             dst=Path(self.package_folder) / "licenses")
        for component, license_file in (
            ("briefpp", "LICENSE"),
            ("cpp-httplib", "LICENSE"),
            ("glfw", "LICENSE.md"),
            ("imgui", "LICENSE.txt"),
        ):
            copy(self, license_file,
                 src=Path(self.source_folder) / "external" / component,
                 dst=Path(self.package_folder) / "licenses" / component)

    #
    # Information for consumers
    #
    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "snt")
        self.cpp_info.set_property("cmake_target_name", "snt::snt")

        self.cpp_info.libs = [
            "snt-api",
            "snt-dip",
            "snt-puq",
            "snt-val",
            "snt-exs",
            "snt-core",
        ]

        components = {
            "core": ("snt-core", []),
            "exs": ("snt-exs", []),
            "val": ("snt-val", ["core"]),
            "puq": ("snt-puq", ["core", "exs", "val"]),
            "dip": ("snt-dip", ["puq", "hdf5::hdf5_c"]),
            "api": ("snt-api", ["puq", "dip"]),
        }

        for name, (library, requirements) in components.items():
            component = self.cpp_info.components[name]
            component.libs = [library]
            component.requires = requirements
            component.set_property("cmake_target_name", f"snt::{name}")
