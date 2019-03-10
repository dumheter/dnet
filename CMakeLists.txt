cmake_minimum_required(VERSION 3.10)
project(dnet)

set(CMAKE_CXX_STANDARD 17)

include_directories(
  .
  thirdparty/chif_net

  # only for examples
  examples/thirdparty/fmt/include
  examples/thirdparty/argparse
)

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

set(COMMON_SOURCE
  dnet/connection.hpp
  dnet/header/packet_header.hpp
  dnet/payload/payload.hpp
  dnet/socket/socket.cpp
  dnet/socket/socket.hpp
  dnet/transport/tcp.cpp
  dnet/transport/tcp.hpp
  dnet/transport/udp.cpp
  dnet/transport/udp.hpp
  dnet/util/macros.hpp
  dnet/util/types.hpp
  dnet/util/platform.hpp
  dnet/util/util.hpp
  dnet/util/util.cpp

  thirdparty/chif_net/chif_net/chif_net.c
  thirdparty/chif_net/chif_net/chif_net.h

  )

set(IO_SOURCE
  examples/thirdparty/argparse/argparse.c
  examples/thirdparty/argparse/argparse.h

  examples/thirdparty/fmt/src/format.cc
  examples/thirdparty/fmt/include/fmt/format.h

  )

add_executable(echo_server examples/echo_server.cpp ${COMMON_SOURCE} ${IO_SOURCE})
add_executable(echo_client examples/echo_client.cpp ${COMMON_SOURCE} ${IO_SOURCE})
add_executable(custom_header_data examples/custom_header_data.cpp ${COMMON_SOURCE} ${IO_SOURCE})

if (WIN32)
  target_link_libraries(echo_server ws2_32)
  target_link_libraries(echo_client ws2_32)
  target_link_libraries(custom_header_data ws2_32)
else()
  target_link_libraries(echo_server pthread)
  target_link_libraries(echo_client pthread)
  target_link_libraries(custom_header_data pthread)
endif()
