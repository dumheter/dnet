// ============================================================ //
// Headers
// ============================================================ //

#include <net/transport/tcp.hpp>
#include <iostream>
#include <thread>
#include <memory>
#include <net/connection/connection.hpp>
#include <net/header/header.hpp>

using namespace dnet;

// ============================================================ //
// Class Definition
// ============================================================ //

void serve(std::unique_ptr<Connection<Tcp, Header>>&& client)
{
  try {
    bool run = true;
    payload_container payload;
    while (run) {
      client->read(payload);
      static_assert(sizeof(u8) == sizeof(char));
      std::cout << "[serve:" << payload.size() << ":";
      for (auto c : payload)
        std::cout << static_cast<char>(c);
      std::cout << "]\n";

      client->write(payload);
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
    Connection<Tcp, Header> server{};
    server.start_server(1337);

    bool run = true;
    while (run) {
      std::cout << "[server: waiting for client]\n";
      auto client = server.accept();
      std::cout << "[server: new client] ["
                << client.get_remote_ip() << ":"
                << client.get_remote_port() << "]\n";

      auto cli_ptr = std::make_unique<Connection<Tcp, Header>>(std::move(client));
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
