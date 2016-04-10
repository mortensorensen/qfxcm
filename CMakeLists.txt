cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR) 
 
####################################################
# CONFIGURATION
####################################################
set (PROGRAM_NAME     "qfxcm")
set (PROGRAM_VER      "0.0.1")
set (KX_VER           "3")
set (FOREXCONNECT_VER "1.4.1")
set (BINARY_NAME      "${PROGRAM_NAME}.${PROGRAM_VER}")

option (BUILD_x86   "build a 32-bit binary instead of 64-bit"               ON)
option (BUILD_DEBUG "build debug versions of the binaries with symbols"     ON)

project (${BINARY_NAME} CXX C)

set (FOREXCONNECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ForexConnectAPI-1.4.1-Darwin-i386")
set (SAMPLE_TOOLS_DIR "${FOREXCONNECT_DIR}/samples/cpp/sample_tools")

set (CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")

configure_file ("${CMAKE_SOURCE_DIR}/config.h.in" "${CMAKE_BINARY_DIR}/include/config.h")

file (GLOB_RECURSE QFXCM_SRC
  "${CMAKE_SOURCE_DIR}/src/*.h"
  "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_library (${BINARY_NAME} SHARED ${QFXCM_SRC})
# add_library (${BINARY_NAME} SHARED "third_party/kx/k.h" "src/stdafx.h" "src/Helpers.h" "src/qforexconnect.cpp")

include_directories ("${CMAKE_BINARY_DIR}/include"
                     "${FOREXCONNECT_DIR}/include"
                     "${SAMPLE_TOOLS_DIR}/include"
                     "${CMAKE_SOURCE_DIR}/third_party/kx")

add_definitions (-DKXVER=3)

find_library (FOREXCONNECT_LIB ForexConnect "${FOREXCONNECT_DIR}/lib")
find_library (SAMPLE_TOOLS_LIB sample_tools "${SAMPLE_TOOLS_DIR}/lib")

target_link_libraries (${BINARY_NAME} ${FOREXCONNECT_LIB} ${SAMPLE_TOOLS_LIB})

set_target_properties (${BINARY_NAME} PROPERTIES
    PREFIX ""
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)

if (DEFINED Mac32)
    set (ARCHFLAGS "-arch i386 -DMac32")
elseif (${CMAKE_SYSTEM} MATCHES "Linux" AND ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i[3-6]86")
    set (ARCHFLAGS "-march=i486")
else ()
    set (ARCHFLAGS "-arch i386")
endif ()

set (CMAKE_OSX_ARCHITECTURES i386) # TODO: Improve

set (CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCHFLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCHFLAGS}")

if (${CMAKE_SYSTEM} MATCHES "Linux")
    set_target_properties (${BINARY_NAME} PROPERTIES
          COMPILE_FLAGS "-fPIC"
          LINK_FLAGS "-shared -Wl,-rpath,.")
elseif (${CMAKE_SYSTEM} MATCHES "Darwin")
    set_target_properties (${BINARY_NAME} PROPERTIES
          BUILD_WITH_INSTALL_RPATH 1
          INSTALL_NAME_DIR "@rpath"
          # LINK_FLAGS "-m32 -bundle -undefined dynamic_lookup -Wl,-rpath,@loader_path,-rpath,./"
          LINK_FLAGS "-undefined dynamic_lookup -Wl,-rpath,@loader_path,-rpath,./"
          SUFFIX ".so")
endif ()

execute_process (COMMAND
  "git" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_SHA1
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process (COMMAND
  "git" log -1 --format=%ad --date=local
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process (COMMAND
  "date"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE BUILD_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# Post - build event to create symlinks for shared objects
# add_custom_command (TARGET ${BINARY_NAME}
#     POST_BUILD
#     COMMAND find ${FOREXCONNECT_DIR}/lib/ -name lib*.${SHAREDSUFFIX} -exec ln -sf {} \;
#     COMMAND find ${SAMPLE_TOOLS_DIR}/lib/ -name lib*.${SHAREDSUFFIX} -exec ln -sf {} \;

#     WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
#     VERBATIM)

# configure_file ("${CMAKE_SOURCE_DIR}/README.md" "${CMAKE_SOURCE_DIR}/build/README.md")
# configure_file ("${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/build/LICENSE")
# configure_file ("${CMAKE_SOURCE_DIR}/src/config/qfxcm.q" "${CMAKE_SOURCE_DIR}/build/qfxcm.q")
# configure_file ("${CMAKE_SOURCE_DIR}/src/config/qfxcm_test.q" "${CMAKE_SOURCE_DIR}/build/qfxcm_test.q")
# configure_file ("${CMAKE_SOURCE_DIR}/Debug/${BINARY_NAME}.so" "${CMAKE_SOURCE_DIR}/build/${BINARY_NAME}.so")

# add_custom_target (build_package COMMAND
#   ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_SOURCE_DIR}/${PROGRAM_NAME}-${PROGRAM_VER}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.tar.gz"
#   "${CMAKE_BINARY_DIR}/${BINARY_NAME}.so"
#   "${CMAKE_BINARY_DIR}/README.md"
#   "${CMAKE_BINARY_DIR}/LICENSE"
#   "${CMAKE_BINARY_DIR}/qfxcm.q")