vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO vrtulka23/scinumtools3
    REF v0.8.0
    SHA512 185745f10fd603671c054d41e0351f4d8b16994955dfbe5fb35f4b25c34820fbacbf0e430d74701c5bf99f94eae1b7aedbff3e2fba3b1082bc9c8f83718fa396
)

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
        -DENABLE_SNT_VIEW=OFF
        -DSNT_HTTPLIB_INCLUDE_DIR=${CURRENT_INSTALLED_DIR}/include
        -DCMAKE_INSTALL_BINDIR=tools/${PORT}
        -DENABLE_EXEC_APPS_DMAP=OFF
        -DENABLE_EXEC_EXAMPLES=OFF
        -DENABLE_EXEC_BENCHMARKS=OFF
    OPTIONS_DEBUG
        -DENABLE_EXEC_APPS=OFF
        -DENABLE_EXEC_APPS_SNT=OFF
        -DENABLE_SNT_SERVER=OFF
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
