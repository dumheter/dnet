cmake_minimum_required(VERSION 3.10)
project(net)

set(CMAKE_CXX_STANDARD 17)

include_directories(
  source
  source/thirdparty
)

set(COMMON_SOURCE
  source/thirdparty/chif/chif_net.h

  source/net/socket/socket.cpp
  source/net/socket/socket.hpp

  source/net/transport/tcp.cpp
  source/net/transport/tcp.hpp
  source/net/transport/udp.cpp
  source/net/transport/udp.hpp

  source/net/util/dnet_exception.hpp
  source/net/util/macros.hpp
  source/net/util/types.hpp
  source/net/util/platform.hpp

  )

add_executable(basic source/examples/basic.cpp ${COMMON_SOURCE})
add_executable(echo_server source/examples/echo_server.cpp ${COMMON_SOURCE})
add_executable(echo_client source/examples/echo_client.cpp ${COMMON_SOURCE})

target_link_libraries(echo_server pthread)
target_link_libraries(echo_client pthread)