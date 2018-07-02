// ============================================================ //
// Headers
// ============================================================ //

#include <net/transport/tcp.hpp>
#include <iostream>
#include <thread>

using namespace dnet;

// ============================================================ //
// Class Definition
// ============================================================ //

int main()
{
  static_assert(sizeof(u8) == sizeof(char));

  try {
    std::string msg("hey");

    Tcp client{};
    client.connect("127.0.0.1", 1337);
    client.write(reinterpret_cast<u8*>(&(msg[0])), msg.size());
    std::cout << "[wrote: " << msg << "]\n";

    constexpr u32 buf_size = 1024;
    u8 buf[buf_size];
    auto bytes = client.read(buf, buf_size);
    std::cout << "[read: " << std::string(reinterpret_cast<char*>(buf), bytes)
              << "]\n";
  }
  catch (const dnet_exception& e) {
    std::cout << "[ex: " << e.what() << "]\n";
  }

  return 0;
}
