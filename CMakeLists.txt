cmake_minimum_required(VERSION 3.10)
project(dnet)
set(CMAKE_CXX_STANDARD 17)

include_directories(
  .
  )

# example & test
include_directories(SYSTEM

  thirdparty/fmt/include
  thirdparty/argparse
  thirdparty/doctest/doctest
  thirdparty#/dutil & /dlog
)

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

set(COMMON_SOURCE
  dnet/tcp_connection.hpp
  dnet/network_handler.hpp
  dnet/net/packet_header.hpp
  dnet/net/socket.cpp
  dnet/net/socket.hpp
  dnet/net/tcp.cpp
  dnet/net/tcp.hpp
  dnet/net/udp.cpp
  dnet/net/udp.hpp
  dnet/util/macros.hpp
  dnet/util/types.hpp
  dnet/util/platform.hpp
  dnet/util/util.hpp
  dnet/util/util.cpp
  dnet/util/dnet_assert.cpp
  dnet/util/dnet_assert.hpp
  )

include_directories(SYSTEM
  thirdparty/chif_net
  )
add_subdirectory(thirdparty/chif_net)

set(IO_SOURCE
  thirdparty/argparse/argparse.c
  thirdparty/fmt/src/format.cc
  )
aux_source_directory(thirdparty/dutil COMMON_SOURCE)
aux_source_directory(thirdparty/dlog IO_SOURCE)

aux_source_directory(test TEST_SOURCE)

add_library(${PROJECT_NAME} STATIC  ${COMMON_SOURCE})

if (DNET_BUILD_EXAMPLES)
add_executable(echo_server examples/echo_server.cpp ${COMMON_SOURCE}
  ${IO_SOURCE})
add_executable(echo_client examples/echo_client.cpp ${COMMON_SOURCE}
  ${IO_SOURCE})
add_executable(custom_header_data examples/custom_header_data.cpp
  ${COMMON_SOURCE} ${IO_SOURCE})
add_compile_definitions(coustom_header_data DLOG_MT DLOG_TIMESTAMP)
endif ()

if (DNET_BUILD_TESTS)
add_executable(test ${TEST_SOURCE} ${IO_SOURCE})
endif ()

if (WIN32)
  if (DNET_BUILD_EXAMPLES)
    target_link_libraries(echo_server chif_net ws2_32)
    target_link_libraries(echo_client chif_net ws2_32)
    target_link_libraries(custom_header_data chif_net ws2_32)
  endif ()
  if (DNET_BUILD_TESTS)
    target_link_libraries(test ${PROJECT_NAME} chif_net ws2_32)
  endif ()
else()
  if (DNET_BUILD_EXAMPLES)
    target_link_libraries(echo_server chif_net pthread)
    target_link_libraries(echo_client chif_net pthread)
    target_link_libraries(custom_header_data chif_net pthread)
  endif()
  if (DNET_BUILD_TESTS)
    target_link_libraries(test chif_net ${PROJECT_NAME} pthread)
  endif ()
endif()
