// ============================================================ //
// Headers
// ============================================================ //

#include <net/transport/tcp.hpp>
#include <iostream>
#include <thread>
#include <memory>

using namespace dnet;

// ============================================================ //
// Class Definition
// ============================================================ //

void serve(std::unique_ptr<Tcp>&& client)
{
  try {
    bool run = true;
    constexpr u32 buf_size = 1024;
    u8 buf[buf_size];
    while (run) {
      auto bytes = client->read(buf, buf_size);
      static_assert(sizeof(u8) == sizeof(char));
      std::cout << "[serve: " << std::string(reinterpret_cast<char*>(buf), bytes) << "]\n";
      bytes = client->write(buf, bytes);
    }
  }
  catch (const dnet_exception& e) {
    std::cout << "[serve: ex: " << e.what() << "]\n";
  }
}

int main()
{
  startup();

  try {
    Tcp server{};
    server.start_server(1337);

    bool run = true;
    while (run) {
      std::cout << "[server: waiting for client]\n";
      auto client = server.accept();
      std::cout << "[server: new client] ["
                << client.get_remote_ip() << ":"
                << client.get_remote_port() << "]\n";

      auto cli_ptr = std::make_unique<Tcp>(std::move(client));
      std::thread t(serve, std::move(cli_ptr));
      t.detach();
    }
  }
  catch (const dnet_exception& e) {
    std::cout << "[ex: " << e.what() << "]\n";
  }

  shutdown();

  return 0;
}
