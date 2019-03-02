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

void die_on_fail(dnet::Result res, dnet::Connection<dnet::Tcp>& con) {
  if (res == dnet::Result::kFail) {
    dprint("failed with error [{}]\n", con.last_error_to_string());
    abort();
  }
}

// ============================================================ //

void serve(std::unique_ptr<dnet::Connection<dnet::Tcp>>&& client)
{
  const auto maybe_port = client->get_port();
  if (maybe_port.has_value()) {
    const u16 port = maybe_port.value();
    dnet::payload_container payload;
    bool run = true;
    dnet::Result res;
    while (run) {

      res = client->read(payload);
      if (res == dnet::Result::kSuccess) {
        dprint("[serve:{}] [msg:{}:", port, payload.size());
        for (const auto c : payload) {
          dprintclean("{}", static_cast<char>(c));
        }
        dprintclean("]\n");

        res = client->write(payload);
        if (res != dnet::Result::kSuccess) {
          dprint("failed to write with error [{}]\n", client->last_error_to_string());
          client->disconnect();
          run = false;
        }
      }
      else {
        dprint("failed to read with error [{}]\n", client->last_error_to_string());
        client->disconnect();
        run = false;
      }
    }
  }
  else {
    dprint("failed to get port with error [{}]\n", client->last_error_to_string());
    client->disconnect();
  }
}

void run_server(u16 port)
{
  dnet::Connection<dnet::Tcp> server{};

  dnet::Result res = server.start_server(port);
  die_on_fail(res, server);
  dprint("init server @ {}:{}\n", server.get_ip().value_or("error"), server.get_port().value_or(0));

  bool run = true;
  while (run) {
    dprint("waiting for client\n");
    auto maybe_client = server.accept();
    if (maybe_client.has_value()) {
      dprint("new client from {}:{}\n", maybe_client.value().get_ip().value_or("error"),
             maybe_client.value().get_port().value_or(0));
      auto cli_ptr = std::make_unique<dnet::Connection<dnet::Tcp>>(std::move(maybe_client.value()));
      std::thread t(serve, std::move(cli_ptr));
      t.detach();
    }
    else {
      dprint("failed to accept client with error [{}]\n", server.last_error_to_string());
    }
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
  run_server(static_cast<u16>(port));
  dnet::shutdown();

  return 0;
}
