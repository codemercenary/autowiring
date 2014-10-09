find_package(Boost COMPONENTS regex system QUIET)

add_googletest(test)
set(AutoNet_SRCS
  AutoNetServer.cpp
  AutoNetServer.h
  AutoNetServerImpl.cpp
  AutoNetServerImpl.hpp
  ${PROJECT_SOURCE_DIR}/contrib/json11/json11.cpp
  ${PROJECT_SOURCE_DIR}/contrib/json11/json11.hpp
)

if(NOT Boost_FOUND)
  if(WIN32)
    # We can't find boost, but whatever.  We know it's available on nuget.
    list(APPEND AutoNet_SRCS packages.config)

    # Boost libraries will be somewhere else other than where we expect
    set(Boost_FOUND ON)
    set(Boost_LIBRARIES)
    set(Boost_INCLUDE_DIR "${CMAKE_BINARY_DIR}/packages/boost.1.56.0.0/lib/native/include")
  else()
    message("Cannot build Autonet, boost not installed on this system")
    return()
  endif()
endif()

option(AUTOWIRING_BUILD_AUTONET "Build Autonet debugging server" ${AUTOWIRING_BUILD_AUTONET_DEFAULT})
if(NOT AUTOWIRING_BUILD_AUTONET)
  return()
endif()

# All include files are located in /autowiring from here, so prepend that to all sources
rewrite_header_paths(AutoNet_SRCS)
ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" AutoNet_SRCS)

include_directories(${Boost_INCLUDE_DIR})

add_library(AutoNet STATIC ${AutoNet_SRCS})
target_link_libraries(AutoNet Autowiring ${Boost_LIBRARIES})
set_property(TARGET AutoNet PROPERTY FOLDER "Autowiring")

#
# Install library
#

if(NOT AUTOWIRING_IS_EMBEDDED)
  install(TARGETS AutoNet EXPORT AutowiringTargets
    DESTINATION lib
    COMPONENT autowiring 
    CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
  )
endif()
