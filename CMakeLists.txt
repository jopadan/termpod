cmake_minimum_required(VERSION 3.0)
project(podorgana LANGUAGES C)

set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION POD game archive format library written in C for all existing versions)

include(GNUInstallDirs)
include(FindPkgConfig)

set(SOURCE_FILES
	src/ccitt32_crc.c
	src/libpodfmt.c
	src/pod_common.c
	src/pod1.c
	src/pod2.c
	src/pod3.c
	src/pod4.c
	src/pod5.c
	src/pod6.c
	src/epd.c
)

set(INCLUDE_FILES
	src/ccitt32_crc.h
	src/libpodfmt.h
	src/pod_common.h
	src/pod1.h
	src/pod2.h
	src/pod3.h
	src/pod4.h
	src/pod5.h
	src/pod6.h
	src/epd.h
)

set(EXAMPLE_SOURCE_FILES
)

# libpodfmt library
add_library( podfmt SHARED ${SOURCE_FILES} ${INCLUDE_FILES} )
target_include_directories(podfmt PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
			$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libpodfmt>
			)

set(install_prefix "${CMAKE_INSTALL_PREFIX}")
set(install_libdir "${CMAKE_INSTALL_LIBDIR}")
configure_file(libpodfmt.pc.in ${PROJECT_BINARY_DIR}/libpodfmt.pc @ONLY)

set(ENV{PKG_CONFIG_PATH} "${PROJECT_BINARY_DIR}:$ENV{PKG_CONFIG_PATH}")

install(TARGETS podfmt EXPORT ${PROJECT_NAME}Config
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libpodfmt
	)

install(FILES ${INCLUDE_FILES} DESTINATION
	${CMAKE_INSTALL_INCLUDEDIR}/libpodfmt/)
install(FILES libpodfmt.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/)
