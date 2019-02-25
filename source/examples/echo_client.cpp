// ============================================================ //
// Headers
// ============================================================ //

#include "dnet/transport/tcp.hpp"
#include "dnet/header/packet_header.hpp"
#include "dnet/connection.hpp"
#include "dnet/util/util.hpp"
#include "fmt/format.h"
#include "argparse.h"
#include <iostream>
#include <thread>

// ============================================================ //
// Debug
#define DNET_DEBUG

#ifdef DNET_DEBUG
#  define dprint(...) \
fmt::print("[client] [{}:{}] ", __func__, __LINE__); \
fmt::print(__VA_ARGS__)
#else
#  define dprintln(...)
#  define dprint(...)
#endif

// ============================================================ //

void echo_client(u16 port, const char* ip)
{
  try {
    dprint("connecting to {}:{}\n", ip, port);
    std::string msg("hey there from the client");
    dnet::payload_container payload{msg.begin(), msg.end()};

    dnet::Connection<dnet::Tcp> client{};
    client.connect(std::string(ip), port);
    client.write(payload);
    dprint("[{}] [wrote:{}]\n", __func__, msg);

    payload.clear();
    client.read(payload);
    dprint("[read:{}]\n", std::string(payload.begin(), payload.end()));
  }
  catch (const dnet::dnet_exception& e) {
    dprint("[ex:{}]\n", e.what());
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
    OPT_INTEGER('p', "port", &port, "port"),
    OPT_STRING('i', "ip", &ip, "ip address"),
    OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, NULL, 0);
  argparse_describe(&argparse, NULL, NULL);
  argc = argparse_parse(&argparse, argc, argv);

  if (!ip)
    ip = "127.0.0.1";

  dnet::startup();
  echo_client(port, ip);
  dnet::shutdown();

  char f;
  std::cin >> f;
  return 0;
}
