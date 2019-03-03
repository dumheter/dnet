// ============================================================ //
// Headers
// ============================================================ //

#include "dnet/connection.hpp"
#include "dnet/header/packet_header.hpp"
#include "dnet/transport/tcp.hpp"
#include "dnet/util/util.hpp"
#include "fmt/format.h"
#include "argparse.h"
#include <thread>
#include <string>
#include <cstdlib>
#include <iostream>

// ============================================================ //
// Debug
#define DNET_DEBUG

#ifdef DNET_DEBUG
#define dprint(...)                                    \
  fmt::print("[client] [{}:{}] ", __func__, __LINE__); \
  fmt::print(__VA_ARGS__)
#else
#define dprintln(...)
#define dprint(...)
#endif

// ============================================================ //

void die_on_fail(dnet::Result res, dnet::Connection<dnet::Tcp>& con) {
  if (res == dnet::Result::kFail) {
    dprint("failed with error [{}]\n", con.last_error_to_string());
    abort();
  }
}

// ============================================================ //

void echo_client(u16 port, const char* ip) {
  dprint("connecting to {}:{}\n", ip, port);
  std::string msg("Hey there, from the client.");
  dnet::payload_container payload{msg.begin(), msg.end()};

  dnet::Connection<dnet::Tcp> client{};

  dnet::Result res = client.connect(std::string(ip), port);
  die_on_fail(res, client);

  dnet::Header_data_example header_data{};
  res = client.write(payload, header_data);
  die_on_fail(res, client);
  dprint("[{}] [wrote:{}]\n", __func__, msg);

  payload.clear();
  auto [read_res, header_data_read] = client.read(payload);
  die_on_fail(read_res, client);
  dprint("[read:{}]\n", std::string(payload.begin(), payload.end()));
}

// ============================================================ //

int main(int argc, const char** argv) {
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

  if (!ip) ip = "127.0.0.1";

  if (port < std::numeric_limits<u16>::min() ||
      port > std::numeric_limits<u16>::max()) {
    dprint("invalid port provided, must be in the range [{}-{}]\n",
           std::numeric_limits<u16>::min(), std::numeric_limits<u16>::max());
    return 1;
  }

  dnet::startup();
  echo_client(static_cast<u16>(port), ip);
  dnet::shutdown();

  fmt::print("enter any key to exit\n> ");
  char f;
  std::cin >> f;
  return 0;
}
