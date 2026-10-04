vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO vrtulka23/scinumtools3
    REF v0.9.0
    SHA512 90a8f9764e4f06ef51399c41b3c2ac0ed062dad19e2bb996f4b9af06a3f5b097a6fdde7985a6deaf757ad907fc847024ef5281721fd02e53ca0d9f11b39d539d
    # Remove these patches when REF points to a release containing both changes.
    PATCHES
        fix-diph5-size-t.patch
        optional-report.patch
)

if("reports" IN_LIST FEATURES)
    vcpkg_from_github(
        OUT_SOURCE_PATH BRIEFPP_SOURCE_PATH
        REPO vrtulka23/briefpp
        REF 624aa478149a0fa0e7213bb7cfb6d19615d6e771
        SHA512 1697cef47f04dd2105f0026ff36ee200ea86e450e38f58403d11f5eb213e82bba2b5b16d1d887599b518f0e1906a1cc56aebc6f76bb5cc7e150603b6a55476a1
    )
    set(REPORT_OPTIONS
        -DENABLE_SNT_REPORT=ON
        -DSNT_BRIEFPP_INCLUDE_DIR=${BRIEFPP_SOURCE_PATH}/include
    )
else()
    set(REPORT_OPTIONS -DENABLE_SNT_REPORT=OFF)
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH IMGUI_SOURCE_PATH
    REPO ocornut/imgui
    REF f1cc2ae15e53a861a874c3034aae6798fde194ab
    SHA512 ca47e0f8a80c0518a42e78d95f87611695e3b888640cf51ce686350c93f65840f2da15c6f412264e1fe62c12aa0cbefbf554bd8fc89cdaeff50c42431fe347fb
)

vcpkg_from_github(
    OUT_SOURCE_PATH GLFW_SOURCE_PATH
    REPO glfw/glfw
    REF d9d6f0f1f967807ffade6598ea9a631ebaf37a56
    SHA512 e41bbc7bdd727c2e36653f4b529879ab7dc48364ba8ac73173ae471274d223ea74e8699cdd2cadad61ae856cd734b0448c1dfb2628449d58bee11c754e740212
)

# GitHub archives omit submodules; the viewer expects these source paths.
file(COPY "${IMGUI_SOURCE_PATH}/" DESTINATION "${SOURCE_PATH}/external/imgui")
file(COPY "${GLFW_SOURCE_PATH}/" DESTINATION "${SOURCE_PATH}/external/glfw")

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DENABLE_UNIT_TESTS=OFF
        -DENABLE_BINDING_PYTHON=OFF

        -DENABLE_CORE=ON
        -DENABLE_EXS=ON
        -DENABLE_VAL=ON
        -DENABLE_PUQ=ON
        -DENABLE_DIP=ON
        -DENABLE_MAT=OFF
        -DENABLE_API=ON

        -DENABLE_EXEC_APPS=ON
        -DENABLE_EXEC_APPS_SNT=ON
        -DENABLE_SNT_SERVER=ON
        -DENABLE_SNT_VIEW=ON
        ${REPORT_OPTIONS}
        # Keep the bundled GLFW inside snt and use the X11 backend on Linux.
        -DGLFW_LIBRARY_TYPE=STATIC
        -DGLFW_BUILD_WAYLAND=OFF
        -DSNT_HTTPLIB_INCLUDE_DIR=${CURRENT_INSTALLED_DIR}/include
        -DCMAKE_INSTALL_BINDIR=tools/${PORT}
        -DENABLE_SNT_DMAP=ON
        -DENABLE_EXEC_EXAMPLES=OFF
        -DENABLE_EXEC_BENCHMARKS=OFF
    OPTIONS_DEBUG
        -DENABLE_EXEC_APPS=OFF
        -DENABLE_EXEC_APPS_SNT=OFF
        -DENABLE_SNT_SERVER=OFF
        -DENABLE_SNT_VIEW=OFF
        -DENABLE_SNT_DMAP=OFF
)

vcpkg_cmake_install()

# Install directly to tools so exported executable paths stay valid.
vcpkg_copy_tool_dependencies("${CURRENT_PACKAGES_DIR}/tools/${PORT}")

vcpkg_cmake_config_fixup(
    PACKAGE_NAME snt
    CONFIG_PATH lib/cmake/snt
)

# Headers should only be installed once.
file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
)

file(INSTALL
    "${SOURCE_PATH}/LICENSE"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright
)
