cmake_minimum_required (VERSION 2.6) 
 
set (PROJECT qfxcm)

project (${PROJECT}) 

if (DEFINED Mac32)
    set (ARCHFLAGS "-arch i386 -DMac32")
elseif (${CMAKE_SYSTEM} MATCHES "Linux" AND ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i[3-6]86")
    set (ARCHFLAGS "-march=i486")
else ()
    set (ARCHFLAGS "-arch i386")
endif ()

add_definitions (-DKXVER=3)

set (SHAREDSUFFIX "so")

set (CMAKE_OSX_ARCHITECTURES i386)

set (SAMPLE_TOOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ForexConnectAPI/samples/cpp/sample_tools")
set (FOREXCONNECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ForexConnectAPI")

set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${ARCHFLAGS}")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${ARCHFLAGS}")

set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} ${ARCHFLAGS}")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${ARCHFLAGS}")

set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS} ${ARCHFLAGS}")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} ${ARCHFLAGS}")

# Default build mode
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release CACHE STRING
         "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
         FORCE
    )
endif (NOT CMAKE_BUILD_TYPE)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")


include_directories (${FOREXCONNECT_DIR}/include
                     ${SAMPLE_TOOLS_DIR}/include
                    )   

link_directories (/usr/local/lib)

set (HEADERS 
    source/stdafx.h
    source/CommonSources.h
    source/ResponseListener.h
    source/SessionStatusListener.h
    source/LoginParams.h
    source/k.h
    source/MarketDataAccumulator.h
    source/Helpers.h
    source/Wrapper.h
    )
     
set (SOURCES
    source/CommonSources.cpp
    source/ResponseListener.cpp
    source/SessionStatusListener.cpp
    source/LoginParams.cpp
    source/MarketDataAccumulator.cpp
    source/Helpers.cpp
    source/Wrapper.cpp
    ) 

add_library (${PROJECT} SHARED ${HEADERS} ${SOURCES})

if (${CMAKE_SYSTEM} MATCHES "Linux")
    target_link_libraries (${PROJECT} ForexConnect sample_tools)
elseif (${CMAKE_SYSTEM} MATCHES "Darwin")
    target_link_libraries (${PROJECT} ForexConnect sample_tools)
    # target_link_libraries (${PROJECT} ForexConnect)
endif ()

# Post - build event to create symlinks for shared objects
add_custom_command (TARGET ${PROJECT}
    POST_BUILD
    COMMAND find ${FOREXCONNECT_DIR}/lib/ -name lib*.${SHAREDSUFFIX} -exec ln -sf {} \;
    COMMAND find ${SAMPLE_TOOLS_DIR}/lib/ -name lib*.${SHAREDSUFFIX} -exec ln -sf {} \;

    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    VERBATIM
)

if (${CMAKE_SYSTEM} MATCHES "Linux")
    set_target_properties (${PROJECT} PROPERTIES
          COMPILE_FLAGS "-m32 -fPIC"
          LINK_FLAGS "-shared -Wl,-rpath,."
          PREFIX "lib"
    )
elseif (${CMAKE_SYSTEM} MATCHES "Darwin")
    set_target_properties (${PROJECT} PROPERTIES
          BUILD_WITH_INSTALL_RPATH 1
          INSTALL_NAME_DIR "@rpath"
          COMPILE_FLAGS "-m32"
          # LINK_FLAGS "-m32 -bundle -undefined dynamic_lookup -Wl,-rpath,@loader_path,-rpath,./"
          LINK_FLAGS "-m32 -undefined dynamic_lookup -Wl,-rpath,@loader_path,-rpath,./"
          PREFIX "lib"
          SUFFIX ".so"
    )
endif ()
