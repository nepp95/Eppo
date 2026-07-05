vcpkg_from_gitlab(
    OUT_SOURCE_PATH SOURCE_PATH
    GITLAB_URL https://gitlab.com
    REPO nepp95/epposcriptcore
    REF e90517a8b5409dda197ff6e5115deb17c63aed57
    SHA512 ab567f4626da1266eb7c74e7ff446dbbe8c1ef82bacc0f0b839af92df244116ab1a6b1ba64f444e275ab0f58c52f913c700fa6dcdb747782351344c46b1799c2
    HEAD_REF main
    PATCHES
        install-targets.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTING=OFF
        -DBUILD_EXAMPLE=OFF
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

include(CMakePackageConfigHelpers)

set(MANAGED_SOURCE_DIR "share/epposcriptcore/EppoScriptCore.Managed")
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/EppoScriptCoreConfig.cmake.in"
    "${CURRENT_PACKAGES_DIR}/share/epposcriptcore/EppoScriptCoreConfig.cmake"
    INSTALL_DESTINATION "share/epposcriptcore"
    PATH_VARS MANAGED_SOURCE_DIR
)

vcpkg_cmake_config_fixup(CONFIG_PATH "share/epposcriptcore")

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/README.md")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
