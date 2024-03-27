# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BUILD_SOURCE_DIRS "/Users/steve/Documents/CR3D/SliCR-3D-V2;/Users/steve/Documents/CR3D/SliCR-3D-V2/build")
set(CPACK_CMAKE_GENERATOR "Xcode")
set(CPACK_COMPONENTS_ALL "Devel;Unspecified")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "/opt/homebrew/Cellar/cmake/3.28.1/share/cmake/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "Slic3r built using CMake")
set(CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE "ON")
set(CPACK_GENERATOR "STGZ;TGZ;TZ")
set(CPACK_INNOSETUP_ARCHITECTURE "x64")
set(CPACK_INSTALL_CMAKE_PROJECTS "/Users/steve/Documents/CR3D/SliCR-3D-V2/build;Slic3r;ALL;/")
set(CPACK_INSTALL_PREFIX "/usr/local")
set(CPACK_MODULE_PATH "/Users/steve/Documents/CR3D/SliCR-3D-V2/cmake/modules/")
set(CPACK_NSIS_DISPLAY_NAME "SliCR-3D-V2 2.5.59")
set(CPACK_NSIS_DISPLAY_NAME_SET "TRUE")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_PACKAGE_NAME "SliCR-3D-V2 2.5.59")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OBJDUMP_EXECUTABLE "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
set(CPACK_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.2.sdk")
set(CPACK_OUTPUT_CONFIG_FILE "/Users/steve/Documents/CR3D/SliCR-3D-V2/cmake/CPackConfig.cmake")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "/Users/steve/Documents/CR3D/SliCR-3D-V2/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CMake is a build tool")
set(CPACK_PACKAGE_EXECUTABLES "slicr-3d")
set(CPACK_PACKAGE_FILE_NAME "SliCR-3D-V2-2.5.59-Linux-x86_64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SliCR-3D-V2 2.5.59")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "SliCR-3D-V2_2.5.59.3_2024-03-16")
set(CPACK_PACKAGE_NAME "SliCR-3D-V2")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "SliCR-3D-V2")
set(CPACK_PACKAGE_VERSION "2.5.59.3")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "/Users/steve/Documents/CR3D/SliCR-3D-V2/LICENSE")
set(CPACK_RESOURCE_FILE_README "/Users/steve/Documents/CR3D/SliCR-3D-V2/README.md")
set(CPACK_RESOURCE_FILE_WELCOME "/Users/steve/Documents/CR3D/SliCR-3D-V2/README.md")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_GENERATOR "TGZ;TZ")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/Users/steve/Documents/CR3D/SliCR-3D-V2/build/CPackSourceConfig.cmake")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "SliCR-3D-V2-2.5.59.3")
set(CPACK_SOURCE_STRIP_FILES "")
set(CPACK_STRIP_FILES "bin/slicr-3d")
set(CPACK_SYSTEM_NAME "Linux-x86_64")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "Linux-x86_64")
set(CPACK_WIX_SIZEOF_VOID_P "8")
set(CPACK_WIX_UPGRADE_GUID "058245e8-20e0-4a95-9ab7-1acfe17ad511")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "/Users/steve/Documents/CR3D/SliCR-3D-V2/build/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
