cmake_minimum_required(VERSION 3.10)
project(dnet)

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

  source/dnet/socket/socket.cpp
  source/dnet/socket/socket.hpp
  source/dnet/transport/tcp.cpp
  source/dnet/transport/tcp.hpp
  source/dnet/transport/udp.cpp
  source/dnet/transport/udp.hpp
  source/dnet/util/dnet_exception.hpp
  source/dnet/util/macros.hpp
  source/dnet/util/types.hpp
  source/dnet/util/platform.hpp
  source/dnet/connection/connection.hpp
  source/dnet/header/header_interface.hpp
  source/dnet/header/packet_header.cpp
  source/dnet/header/packet_header.hpp
  source/dnet/payload/payload.hpp
  source/dnet/util/util.hpp

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