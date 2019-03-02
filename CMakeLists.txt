cmake_minimum_required(VERSION 3.10)
project(dnet)

set(CMAKE_CXX_STANDARD 17)

include_directories(
  source
  source/thirdparty/chif_net
  source/thirdparty/fmt/include
  source/thirdparty/argparse
)

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

set(COMMON_SOURCE
  source/dnet/connection.hpp
  source/dnet/header/packet_header.cpp
  source/dnet/header/packet_header.hpp
  source/dnet/payload/payload.hpp
  source/dnet/socket/socket.cpp
  source/dnet/socket/socket.hpp
  source/dnet/transport/tcp.cpp
  source/dnet/transport/tcp.hpp
  source/dnet/transport/udp.cpp
  source/dnet/transport/udp.hpp
  source/dnet/util/macros.hpp
  source/dnet/util/types.hpp
  source/dnet/util/platform.hpp
  source/dnet/util/util.hpp
  source/dnet/util/util.cpp

  source/thirdparty/chif_net/chif_net/chif_net.c
  source/thirdparty/chif_net/chif_net/chif_net.h

  )

set(IO_SOURCE
  source/thirdparty/argparse/argparse.c
  source/thirdparty/argparse/argparse.h

  source/thirdparty/fmt/src/format.cc
  source/thirdparty/fmt/include/fmt/format.h

  )

add_executable(echo_server source/examples/echo_server.cpp ${COMMON_SOURCE} ${IO_SOURCE})
add_executable(echo_client source/examples/echo_client.cpp ${COMMON_SOURCE} ${IO_SOURCE})

if (WIN32)
  target_link_libraries(echo_server wsock32 ws2_32)
  target_link_libraries(echo_client wsock32 ws2_32)
else()
  target_link_libraries(echo_server pthread)
  target_link_libraries(echo_client pthread)
endif()
