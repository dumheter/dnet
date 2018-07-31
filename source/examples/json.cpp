// ============================================================ //
// Headers
// ============================================================ //

#include <dnet/transport/tcp.hpp>
#include <dnet/header/packet_header.hpp>
#include <dnet/connection/connection.hpp>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fmt/printf.h>

// ============================================================ //
// functions
// ============================================================ //

void ex1()
{
  try {
    dnet::Connection<dnet::Tcp, dnet::Packet_header> con;
    fmt::printf("connecting...\n");
    con.connect("127.0.0.1", 1337);

    nlohmann::json msg = {
      {"fruit",  "apple"},
      {"number", 8}
    };

    std::vector<u8> packed_msg;
    nlohmann::json::to_cbor(msg, packed_msg);
    fmt::printf("sending json packet:\n%s\n", msg.dump(2));
    con.write(packed_msg);


    fmt::printf("waiting for response\n");
    con.read(packed_msg);
    msg = nlohmann::json::from_cbor(packed_msg);
    fmt::printf("got response:\n%s\n", msg.dump(2));
  }
  catch (const std::exception& e) {
    fmt::printf("fatal exception: %s\n", e.what());
  }
}

// ============================================================ //

void ex2()
{

}

// ====================================================================== //
// main
// ====================================================================== //

int main(int argc, char** argv)
{
  ex1();


  return 0;
}
