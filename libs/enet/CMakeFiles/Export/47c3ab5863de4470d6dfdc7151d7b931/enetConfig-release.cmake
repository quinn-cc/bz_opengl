#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "enet::enet_static" for configuration "Release"
set_property(TARGET enet::enet_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(enet::enet_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libenet.a"
  )

list(APPEND _cmake_import_check_targets enet::enet_static )
list(APPEND _cmake_import_check_files_for_enet::enet_static "${_IMPORT_PREFIX}/lib/libenet.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
