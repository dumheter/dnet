// ============================================================ //
// Headers
// ============================================================ //

#include "dnet/transport/tcp.hpp"
#include "dnet/connection.hpp"
#include "dnet/header/packet_header.hpp"
#include "dnet/util/util.hpp"
#include "fmt/format.h"
#include "argparse.h"
#include <iostream>
#include <thread>
#include <memory>

// ============================================================ //
// Debug
#define DNET_DEBUG

#ifdef DNET_DEBUG
#  define dprint(...)  \
fmt::print("[server] [{}:{}] ", __func__, __LINE__); \
fmt::print(__VA_ARGS__)
#  define dprintclean(...) fmt::print(__VA_ARGS__)
#else
#  define dprintln(...)
#  define dprint(...)
#  define dprintclean(...)
#endif

// ============================================================ //
// Class Definition
// ============================================================ //

void serve(std::unique_ptr<dnet::Connection<dnet::Tcp>>&& client)
{
  const auto port = client->get_remote_port();

  try {
      dnet::payload_container payload;
    bool run = true;
    while (run) {
      client->read(payload);
      dprint("[serve:{0}] [msg:{1}:", port, payload.size());
      for (auto c : payload) {
        dprintclean("{0}", static_cast<char>(c));
      }
      dprintclean("]\n");

      client->write(payload);
    }
  }
  catch (const dnet::dnet_exception& e) {
    dprint("[serve:{0}] connection closed [ex:{1}]\n", port, e.what());
  }
}

void run_server(u16 port)
{
  try {
      dnet::Connection<dnet::Tcp> server{};
    server.start_server(port);
    dprint("init server @ {}:{}\n", server.get_ip(), server.get_port());

    bool run = true;
    while (run) {
      dprint("waiting for client\n");
      auto client = server.accept();
      dprint("new client from {0}:{1}\n", client.get_remote_ip(), client.get_remote_port());

      auto cli_ptr = std::make_unique<dnet::Connection<dnet::Tcp>>(std::move(client));
      std::thread t(serve, std::move(cli_ptr));
      t.detach();
    }
  }
  catch (const dnet::dnet_exception& e) {
    dprint("[ex:{0}]\n", e.what());
  }
}

// ============================================================ //

int main(int argc, const char** argv)
{
  int port = 0;
  const char* ip = NULL;
  struct argparse_option options[] = {
    OPT_HELP(),
    OPT_GROUP("Settings"),
    OPT_INTEGER('p', "port", &port, "port", NULL, 0, 0),
    OPT_STRING('i', "ip", &ip, "ip address", NULL, 0, 0),
    OPT_END(),
  };

  struct argparse argparse{};
  argparse_init(&argparse, options, NULL, 0);
  argparse_describe(&argparse, NULL, NULL);
  argc = argparse_parse(&argparse, argc, argv);

  if (port < std::numeric_limits<u16>::min() ||
      port > std::numeric_limits<u16>::max()) {
    dprint("invalid port provided, must be in the range [{}-{}]\n",
    std::numeric_limits<u16>::min(), std::numeric_limits<u16>::max());
    return 1;
  }

  dnet::startup();
  run_server(port);
  dnet::shutdown();

  return 0;
}
