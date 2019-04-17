#include "doctest.h"
#include "dnet/connection.hpp"

TEST_CASE("tcp") {
  dnet::Connection con{};
  constexpr u16 port = 12021;
  const dnet::Result res = con.start_server(port);
  CHECK(res == dnet::Result::kSuccess);

}
