// ============================================================ //
// Headers
// ============================================================ //

#include <net/transport/tcp.hpp>
#include <iostream>
#include <thread>
#include <net/header/packet_header.hpp>
#include <net/connection/connection.hpp>

using namespace dnet;

// ============================================================ //
// Class Definition
// ============================================================ //

int main()
{
  static_assert(sizeof(u8) == sizeof(char));

  try {
    std::string msg("hey there from the client");
    payload_container payload{msg.begin(), msg.end()};

    Connection<Tcp, Packet_header> client{};
    client.connect("127.0.0.1", 1337);
    client.write(payload);
    std::cout << "[wrote: " << msg << "]\n";

    payload.clear();
    client.read(payload);
    std::cout << "[read: " << std::string(payload.begin(), payload.end())
              << "]\n";
  }
  catch (const dnet_exception& e) {
    std::cout << "[ex: " << e.what() << "]\n";
  }

  return 0;
}
