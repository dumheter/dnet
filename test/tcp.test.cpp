#include <doctest.h>
#include <dnet/connection.hpp>
#include <dnet/util/types.hpp>
#include <dnet/net/tcp.hpp>
#include <vector>


TEST_CASE("tcp") {
  dnet::Connection<std::vector<u8>, dnet::Tcp> con{};
  constexpr u16 port = 12021;
  const dnet::Result res = con.start_server(port);
  CHECK(res == dnet::Result::kSuccess);

}
