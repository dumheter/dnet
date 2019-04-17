// ============================================================ //
// Headers
// ============================================================ //

#include <dnet/connection.hpp>
#include <dnet/net/tcp.hpp>
#include <dnet/net/packet_header.hpp>
#include <dnet/util/util.hpp>
#include <dnet/util/platform.hpp>
#include <fmt/format.h>
#include <argparse.h>
#include <iostream>
#include <thread>
#include <memory>
#include <vector>

// ============================================================ //
// Debug
#define DNET_DEBUG

// print with fmt, prefixed with both function name and line number.
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

using EchoConnection = dnet::Connection<std::vector<u8>, dnet::Tcp>;

// ============================================================ //

// hepler function to crash on result fail
void die_on_fail(dnet::Result res, EchoConnection& con) {
  if (res == dnet::Result::kFail) {
    dprint("failed with error [{}]\n", con.last_error_to_string());
    std::cout << std::endl;  // flush
    exit(0);
  }
}

// ============================================================ //

// when a new client connects, this function will be run on a new thread
void serve(EchoConnection client)
{
  // get port
  const auto [got_peer, peer_ip, peer_port] = client.get_peer();
  if (got_peer != dnet::Result::kSuccess) {
    dprint("failed to get peer address with error [{}]\n",
           client.last_error_to_string());
    return;
  }

  std::vector<u8> payload;

  bool run = true;
  while (run) {

    // wait for message
    dprint("[port:{}] waiting for message\n", peer_port);
    auto [res, header_data] = client.read(payload);
    if (res == dnet::Result::kSuccess) {
      dprint("[port:{}] read {} bytes: [", peer_port, payload.size());
      for (const auto c : payload) {
        dprintclean("{}", static_cast<char>(c));
      }
      dprintclean("]\n");

      // echo back the message
      dprint("[port:{}] echoing back the message\n", peer_port);
      res = client.write(header_data, payload);
      if (res != dnet::Result::kSuccess) {
        dprint("[port:{}] failed to write with error [{}]\n",
               peer_port, client.last_error_to_string());
        run = false;
      }
    }
    else {
      dprint("[port:{}] failed to read with error [{}]\n",
             peer_port, client.last_error_to_string());
      run = false;
    }
  }
}

// listen for connections on main thread
void run_server(u16 port)
{
  // start the server
  dprint("starting server\n");
  EchoConnection server{};
  dnet::Result res = server.start_server(port);
  die_on_fail(res, server);
  dprint("server running @ {}:{}\n", server.get_ip().value_or("error"),
         server.get_port().value_or(0));

  bool run = true;
  while (run) {

    // wait for client to connect
    dprint("waiting for client\n");
    auto maybe_client = server.accept();
    if (maybe_client.has_value()) {
      const auto [got_peer, peer_ip, peer_port] = maybe_client.value().get_peer();
      if (got_peer == dnet::Result::kSuccess) {
        dprint("new client from {}:{}\n", peer_ip, peer_port);

        // handle the client on a seperate thread
        std::thread t(serve, std::move(maybe_client.value()));
        t.detach();
      }
      else {
        dprint("failed to get peer with error [{}]\n",
               server.last_error_to_string());
      }
    }
    else {
      dprint("failed to accept client with error [{}]\n",
             server.last_error_to_string());
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

  // windows will instantly close the terminal window, prevent that
#ifdef DNET_PLATFORM_WINDOWS
  fmt::print("enter any key to exit\n> ");
  char f;
  std::cin >> f;
#endif

  return 0;
}
