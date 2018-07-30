cmake_minimum_required(VERSION 3.10)
project(net)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb")

include_directories(
  source
  source/thirdparty
  source/thirdparty/json/single_include
  source/thirdparty/fmt/include
  source/thirdparty/argparse
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

  source/net/connection/connection.hpp
  source/net/header/header_interface.hpp
  source/net/header/packet_header.cpp
  source/net/header/packet_header.hpp

  source/net/payload/payload.hpp
  source/net/util/util.hpp

  )

set(EXAMPLE_SOURCE
  source/thirdparty/argparse/argparse.c
  source/thirdparty/argparse/argparse.h

  source/thirdparty/fmt/include/fmt/core.h
  source/thirdparty/fmt/include/fmt/folly.h
  source/thirdparty/fmt/include/fmt/format.h
  source/thirdparty/fmt/include/fmt/format-inl.h
  source/thirdparty/fmt/include/fmt/ostream.h
  source/thirdparty/fmt/include/fmt/posix.h
  source/thirdparty/fmt/include/fmt/printf.h
  source/thirdparty/fmt/include/fmt/ranges.h
  source/thirdparty/fmt/include/fmt/time.h
  source/thirdparty/fmt/src/format.cc
  source/thirdparty/fmt/src/posix.cc

  source/thirdparty/json/single_include/nlohmann/json.hpp
  )

add_executable(basic source/examples/basic.cpp ${COMMON_SOURCE} ${EXAMPLE_SOURCE})
add_executable(json source/examples/json.cpp ${COMMON_SOURCE} ${EXAMPLE_SOURCE})
add_executable(echo_server source/examples/echo_server.cpp ${COMMON_SOURCE} ${EXAMPLE_SOURCE})
add_executable(echo_client source/examples/echo_client.cpp ${COMMON_SOURCE} ${EXAMPLE_SOURCE})
add_executable(perf_fn_call source/examples/perf_fn_call.cpp ${EXAMPLE_SOURCE})
add_executable(perf source/examples/perf.cpp ${COMMON_SOURCE} ${EXAMPLE_SOURCE})

target_link_libraries(echo_server pthread)
target_link_libraries(echo_client pthread)
target_link_libraries(perf_fn_call pthread benchmark)
target_link_libraries(perf pthread benchmark)