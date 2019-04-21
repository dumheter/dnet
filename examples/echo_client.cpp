#include <argparse.h>
#include <fmt/format.h>
#include <cstdlib>
#include <dnet/tcp_connection.hpp>
#include <dnet/net/packet_header.hpp>
#include <dnet/net/tcp.hpp>
#include <dnet/util/platform.hpp>
#include <dnet/util/util.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// ============================================================ //
// Debug
#define DNET_DEBUG

// print with fmt, prefixed with both function name and line number.
#ifdef DNET_DEBUG
#define dprint(...)                                    \
  fmt::print("[client] [{}:{}] ", __func__, __LINE__); \
  fmt::print(__VA_ARGS__)
#else
#define dprintln(...)
#define dprint(...)
#endif

// ============================================================ //

using EchoConnection = dnet::TcpConnection<std::vector<u8>>;

// ============================================================ //

// hepler function to crash on result fail
void DieOnFail(dnet::Result res, EchoConnection& con) {
  if (res == dnet::Result::kFail) {
    dprint("failed with error [{}]\n", con.LastErrorToString());
    std::cout << std::endl;  // flush
    exit(0);
  }
}

// ============================================================ //

void EchoClient(u16 port, const char* ip) {
  // connect to the server
  dprint("connecting to {}:{}\n", ip, port);
  EchoConnection client{};
  dnet::Result res = client.Connect(std::string(ip), port);
  DieOnFail(res, client);
  dprint("connected\n");

  // prepare payload and header
  std::string msg("Hey there, from the client.");
  std::vector<u8> payload{msg.begin(), msg.end()};
  dnet::HeaderDataExample header_data{};  // use the default header data

  // send the data
  dprint("writing data\n");
  res = client.Write(header_data, payload);
  DieOnFail(res, client);
  dprint("wrote [{}]\n", msg);

  // read response
  dprint("waiting for response\n");
  auto [read_res, header_data_read] = client.Read(payload);
  DieOnFail(read_res, client);
  dprint("read [{}]\n", std::string(payload.begin(), payload.end()));

  // connection will be closed when client is destructed
}

// ============================================================ //

int main(int argc, const char** argv) {
  int port = 0;
  const char* ip = NULL;
  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_GROUP("Settings"),
      OPT_INTEGER('p', "port", &port, "port", NULL, NULL, 0),
      OPT_STRING('i', "ip", &ip, "ip address", NULL, NULL, 0),
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

  dnet::Startup();
  EchoClient(static_cast<u16>(port), ip);
  dnet::Shutdown();

  // windows will instantly close the terminal window, prevent that
#ifdef DNET_PLATFORM_WINDOWS
  fmt::print("enter any key to exit\n> ");
  char f;
  std::cin >> f;
#endif

  return 0;
}
