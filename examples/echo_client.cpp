#include <argparse.h>
#include <cstdlib>
#include <dlog.hpp>
#include <dnet/net/packet_header.hpp>
#include <dnet/net/tcp.hpp>
#include <dnet/tcp_connection.hpp>
#include <dnet/util/platform.hpp>
#include <dnet/util/util.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// ============================================================ //

using EchoConnection = dnet::TcpConnection<std::vector<u8>>;

// ============================================================ //

// hepler function to crash on result fail
void DieOnFail(dnet::Result res, EchoConnection& con) {
  if (res == dnet::Result::kFail) {
    DLOG_ERROR("failed with error [{}]", con.LastErrorToString());
    std::cout << std::endl;  // flush
    exit(0);
  }
}

// ============================================================ //

void EchoClient(u16 port, const char* ip) {
  // connect to the server
  DLOG_INFO("connecting to {}:{}", ip, port);
  EchoConnection client{};
  dnet::Result res = client.Connect(std::string(ip), port);
  DieOnFail(res, client);
  DLOG_INFO("connected");

  // prepare payload and header
  std::string msg("Hey there, from the client.");
  std::vector<u8> payload{msg.begin(), msg.end()};
  dnet::HeaderDataExample header_data{};  // use the default header data

  // send the data
  DLOG_INFO("writing data");
  res = client.Write(header_data, payload);
  DieOnFail(res, client);
  DLOG_INFO("wrote [{}]", msg);

  // read response
  DLOG_INFO("waiting for response");
  auto [read_res, header_data_read] = client.Read(payload);
  DieOnFail(read_res, client);
  DLOG_INFO("read [{}]", std::string(payload.begin(), payload.end()));

  // connection will be closed when client is destructed
}

// ============================================================ //

int main(int argc, const char** argv) {
  int port = 1337;
  const char* ip = NULL;
  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_GROUP("Settings"),
      OPT_INTEGER('p', "port", &port, "port", NULL, 0, 0),
      OPT_STRING('i', "ip", &ip, "ip address", NULL, 0, 0),
      OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, NULL, 0);
  argparse_describe(&argparse, NULL, NULL);
  argc = argparse_parse(&argparse, argc, argv);

  if (!ip) ip = "127.0.0.1";

  if (port < std::numeric_limits<u16>::min() ||
      port > std::numeric_limits<u16>::max()) {
    DLOG_ERROR("invalid port provided, must be in the range [{}-{}]",
           std::numeric_limits<u16>::min(), std::numeric_limits<u16>::max());
    return 1;
  }

  dnet::Startup();
  EchoClient(static_cast<u16>(port), ip);
  dnet::Shutdown();

  // windows will instantly close the terminal window, prevent that
#ifdef DNET_PLATFORM_WINDOWS
  DLOG_RAW("enter any key to exit\n> ");
  char f;
  std::cin >> f;
#endif

  return 0;
}
