// ============================================================ //
// Headers
// ============================================================ //

#include <iostream>
#include <thread>
#include <memory>
#include <fmt/format.h>
#include <net/transport/tcp.hpp>
#include <net/connection/connection.hpp>
#include <net/header/packet_header.hpp>
#include <net/util/util.hpp>

using namespace dnet;

// ============================================================ //
// Debug
#define DNET_DEBUG

#ifdef DNET_DEBUG
#  define dprint(...) fmt::print(__VA_ARGS__)
#else
#  define dprintln(...)
#  define dprint(...)
#endif

// ============================================================ //
// Class Definition
// ============================================================ //

void serve(std::unique_ptr<Connection<Tcp, Packet_header>>&& client)
{
  const auto port = client->get_remote_port();

  try {
    payload_container payload;
    bool run = true;
    while (run) {
      client->read(payload);
      static_assert(sizeof(u8) == sizeof(char));
      dprint("[serve:{0}] [msg:{1}:", port, payload.size());
      for (auto c : payload)
        dprint("{0}", static_cast<char>(c));
      dprint("]\n");

      client->write(payload);
    }
  }
  catch (const dnet_exception& e) {
    dprint("[serve:{0}] connection closed [ex:{1}]\n", port, e.what());
  }
}

void run_server(u16 port)
{
  try {
    Connection<Tcp, Packet_header> server{};
    server.start_server(port);

    bool run = true;
    while (run) {
      dprint("[server] waiting for client\n");
      auto client = server.accept();
      dprint("[server] new client from {0}:{1}\n", client.get_remote_ip(), client.get_remote_port());

      auto cli_ptr = std::make_unique<Connection<Tcp, Packet_header>>(std::move(client));
      std::thread t(serve, std::move(cli_ptr));
      t.detach();
    }
  }
  catch (const dnet_exception& e) {
    dprint("[server] [ex:{0}]\n", e.what());
  }
}

// ============================================================ //

int main()
{
  startup();
  run_server(1337);
  shutdown();

  return 0;
}
