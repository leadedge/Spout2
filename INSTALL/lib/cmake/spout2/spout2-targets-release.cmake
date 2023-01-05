#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Spout2::Spout" for configuration "Release"
set_property(TARGET Spout2::Spout APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Spout2::Spout PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/Spout.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/Spout.dll"
  )

list(APPEND _cmake_import_check_targets Spout2::Spout )
list(APPEND _cmake_import_check_files_for_Spout2::Spout "${_IMPORT_PREFIX}/lib/Spout.lib" "${_IMPORT_PREFIX}/bin/Spout.dll" )

# Import target "Spout2::Spout_static" for configuration "Release"
set_property(TARGET Spout2::Spout_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Spout2::Spout_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/Spout_static.lib"
  )

list(APPEND _cmake_import_check_targets Spout2::Spout_static )
list(APPEND _cmake_import_check_files_for_Spout2::Spout_static "${_IMPORT_PREFIX}/lib/Spout_static.lib" )

# Import target "Spout2::SpoutDX" for configuration "Release"
set_property(TARGET Spout2::SpoutDX APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Spout2::SpoutDX PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/SpoutDX.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/SpoutDX.dll"
  )

list(APPEND _cmake_import_check_targets Spout2::SpoutDX )
list(APPEND _cmake_import_check_files_for_Spout2::SpoutDX "${_IMPORT_PREFIX}/lib/SpoutDX.lib" "${_IMPORT_PREFIX}/bin/SpoutDX.dll" )

# Import target "Spout2::SpoutLibrary" for configuration "Release"
set_property(TARGET Spout2::SpoutLibrary APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Spout2::SpoutLibrary PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/SpoutLibrary.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/SpoutLibrary.dll"
  )

list(APPEND _cmake_import_check_targets Spout2::SpoutLibrary )
list(APPEND _cmake_import_check_files_for_Spout2::SpoutLibrary "${_IMPORT_PREFIX}/lib/SpoutLibrary.lib" "${_IMPORT_PREFIX}/bin/SpoutLibrary.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
