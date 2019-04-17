cmake_minimum_required(VERSION 3.10)
project(dnet)
set(CMAKE_CXX_STANDARD 17)

include_directories(
  .
  thirdparty/chif_net
  # example & test
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
  dnet/connection.hpp
  dnet/net/packet_header.hpp
  dnet/net/payload.hpp
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

  thirdparty/chif_net/chif_net/chif_net.c
  thirdparty/chif_net/chif_net/chif_net.h
  )

set(TEST_SOURCE
  thirdparty/argparse/argparse.c
  thirdparty/fmt/src/format.cc
  )
aux_source_directory(thirdparty/dutil TEST_SOURCE)
aux_source_directory(thirdparty/dlog TEST_SOURCE)
aux_source_directory(test TEST_SOURCE)

add_library(${PROJECT_NAME} STATIC  ${COMMON_SOURCE})

add_executable(echo_server examples/echo_server.cpp ${COMMON_SOURCE} ${TEST_SOURCE})
add_executable(echo_client examples/echo_client.cpp ${COMMON_SOURCE} ${TEST_SOURCE})
add_executable(custom_header_data examples/custom_header_data.cpp ${COMMON_SOURCE} ${TEST_SOURCE})

aux_source_directory(test TEST_SRCS)

add_executable(test ${TEST_SOURCE} ${COMMON_SOURCE})

if (WIN32)
  target_link_libraries(echo_server ws2_32)
  target_link_libraries(echo_client ws2_32)
  target_link_libraries(custom_header_data ws2_32)
  target_link_libraries(test ws2_32)
else()
  target_link_libraries(echo_server pthread)
  target_link_libraries(echo_client pthread)
  target_link_libraries(custom_header_data pthread)
  target_link_libraries(test pthread)
endif()
