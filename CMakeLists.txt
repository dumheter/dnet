cmake_minimum_required(VERSION 3.10)
project(dnet)
set(CMAKE_CXX_STANDARD 17)

option(DNET_BUILD_TESTS "Build test" ON)
option(DNET_BUILD_EXAMPLES "Build examples" ON)

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

set(DNET_SOURCE
  source/dnet/tcp_connection.hpp
  source/dnet/network_handler.hpp
  source/dnet/net/packet_header.hpp
  source/dnet/net/socket.cpp
  source/dnet/net/socket.hpp
  source/dnet/net/tcp.cpp
  source/dnet/net/tcp.hpp
  source/dnet/net/udp.cpp
  source/dnet/net/udp.hpp
  source/dnet/util/macros.hpp
  source/dnet/util/types.hpp
  source/dnet/util/platform.hpp
  source/dnet/util/util.hpp
  source/dnet/util/util.cpp
  source/dnet/util/dnet_assert.cpp
  source/dnet/util/dnet_assert.hpp
  )

include_directories(SYSTEM thirdparty/chif_net)
add_subdirectory(thirdparty/chif_net)

add_library(${PROJECT_NAME} STATIC ${DNET_SOURCE})

if (DNET_BUILD_EXAMPLES OR DNET_BUILD_TESTS)
  add_subdirectory(thirdparty/dlog)
  add_subdirectory(thirdparty/dutil)

  include_directories(SYSTEM
    thirdparty/doctest/doctest
    thirdparty/dlog/source
    thirdparty/dlog/thirdparty
    thirdparty/dutil/source
    thirdparty/argparse
    )

  set(IO_SRC thirdparty/argparse/argparse.c)
endif ()

if (DNET_BUILD_EXAMPLES)
  add_executable(echo_server examples/echo_server.cpp ${DNET_SOURCE} ${IO_SRC})
  add_executable(echo_client examples/echo_client.cpp ${DNET_SOURCE} ${IO_SRC})
  add_executable(custom_header_data examples/custom_header_data.cpp ${DNET_SOURCE} ${IO_SRC})
  #add_compile_definitions(coustom_header_data DLOG_MT DLOG_TIMESTAMP)
endif ()

if (DNET_BUILD_TESTS)
aux_source_directory(test TEST_SOURCE)
add_executable(test ${TEST_SOURCE})
endif ()

# set platform specific libs
if (WIN32)
  set(PLIBS)
else ()
  set(PLIBS pthread)
endif()

if (DNET_BUILD_TESTS)
  target_link_libraries(test ${PROJECT_NAME} ${PLIBS} dlog dutil)
endif ()
if (DNET_BUILD_EXAMPLES)
  target_link_libraries(echo_server ${PROJECT_NAME} ${PLIBS} dlog dutil)
  target_link_libraries(echo_client ${PROJECT_NAME} ${PLIBS} dlog dutil)
  target_link_libraries(custom_header_data ${PROJECT_NAME} ${PLIBS} dlog dutil)
endif ()
target_link_libraries(${PROJECT_NAME} ${PLIBS} chif_net)

target_include_directories(${PROJECT_NAME} PUBLIC source)
