include(GNUInstallDirs)
include(FindPackageHandleStandardArgs)

SET(TERMPOD_FOUND FALSE)

find_path(TERMPOD_INCLUDE_DIR termpod.h
  HINTS
    ENV TERMPODDIR
    PATH_SUFFIXES termpod
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /opt
  ${CMAKE_INSTALL_INCLUDEDIR}
)

MARK_AS_ADVANCED(TERMPOD_INCLUDE_DIR)

message("${TERMPOD_INCLUDE_LIBRARY}")

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TermPOD DEFAULT_MSG TERMPOD_INCLUDE_DIR)

set(TERMPOD_FOUND ${TERMPOD_INCLUDE_LIBRARY}) 
message(${TERMPOD_FOUND})

if(TERMPOD_FOUND)
	set(TERMPOD_INCLUDE_DIRS ${TERMPOD_INCLUDE_DIR})
	if(NOT TARGET TermPOD::termpod)
		add_library(TermPOD::termpod INTERFACE IMPORTED)
		set_target_properties(TermPOD::termpod PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${TERMPOD_INCLUDE_DIR}")
  	endif()
endif()
	
unset(_cmake_find_termpod_output)
