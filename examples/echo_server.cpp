#include <dnet/net/packet_header.hpp>
#include <dnet/net/tcp.hpp>
#include <dnet/tcp_connection.hpp>
#include <dnet/util/platform.hpp>
#include <dnet/util/util.hpp>
#include <argparse.h>
#include <dlog.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// ============================================================ //

using EchoConnection = dnet::TcpConnection<std::vector<u8>>;

// ============================================================ //

// hepler function to crash on result fail
void DieOnFail(dnet::Result res, EchoConnection& con) {
  if (res == dnet::Result::kFail) {
    DLOG_ERROR("failed with error [{}]", con.LastErrorToString());
    dlog::Flush();
    exit(0);
  }
}

// ============================================================ //

// when a new client connects, this function will be run on a new thread
void Serve(EchoConnection client) {
  // get port
  const auto [got_peer, peer_ip, peer_port] = client.GetPeer();
  if (got_peer != dnet::Result::kSuccess) {
    DLOG_ERROR("failed to get peer address with error [{}]",
               client.LastErrorToString());
    return;
  }

  std::vector<u8> payload;

  bool run = true;
  while (run) {
    // wait for message
    DLOG_INFO("[port:{}] waiting for message", peer_port);
    auto [res, header_data] = client.Read(payload);
    if (res == dnet::Result::kSuccess) {
      const auto msg =
          std::string(reinterpret_cast<const char*>(&payload[0]), payload.size());
      DLOG_INFO("[port:{}] read {} bytes: [{}]", peer_port, payload.size(), msg);

      // echo back the message
      DLOG_INFO("[port:{}] echoing back the message", peer_port);
      res = client.Write(header_data, payload);
      if (res != dnet::Result::kSuccess) {
        DLOG_ERROR("[port:{}] failed to write with error [{}]", peer_port,
               client.LastErrorToString());
        run = false;
      }
    } else {
      DLOG_ERROR("[port:{}] failed to read with error [{}]", peer_port,
             client.LastErrorToString());
      run = false;
    }
  }
}

// listen for connections on main thread
void RunServer(u16 port) {
  // start the server
  DLOG_INFO("starting server");
  EchoConnection server{};
  dnet::Result res = server.StartServer(port);
  DieOnFail(res, server);
  DLOG_INFO("server running @ {}:{}", server.GetIp().value_or("error"),
         server.GetPort().value_or(0));

  bool run = true;
  while (run) {
    // wait for client to connect
    DLOG_INFO("waiting for client");
    auto maybe_client = server.Accept();
    if (maybe_client.has_value()) {
      const auto [got_peer, peer_ip, peer_port] =
          maybe_client.value().GetPeer();
      if (got_peer == dnet::Result::kSuccess) {
        DLOG_INFO("new client from {}:{}", peer_ip, peer_port);

        // handle the client on a seperate thread
        std::thread t(Serve, std::move(maybe_client.value()));
        t.detach();
      } else {
        DLOG_ERROR("failed to get peer with error [{}]",
               server.LastErrorToString());
      }
    } else {
      DLOG_ERROR("failed to accept client with error [{}]",
             server.LastErrorToString());
    }
  }
}

// ============================================================ //

int main(int argc, const char** argv) {
  int port = 0;
  const char* ip = NULL;
  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_GROUP("Settings"),
      OPT_INTEGER('p', "port", &port, "port", NULL, 0, 0),
      OPT_STRING('i', "ip", &ip, "ip address", NULL, 0, 0),
      OPT_END(),
  };

  struct argparse argparse {};
  argparse_init(&argparse, options, NULL, 0);
  argparse_describe(&argparse, NULL, NULL);
  argc = argparse_parse(&argparse, argc, argv);

  if (port < std::numeric_limits<u16>::min() ||
      port > std::numeric_limits<u16>::max()) {
    DLOG_ERROR("invalid port provided, must be in the range [{}-{}]",
           std::numeric_limits<u16>::min(), std::numeric_limits<u16>::max());
    return 1;
  }

  dnet::Startup();
  RunServer(static_cast<u16>(port));
  dnet::Shutdown();

  // windows will instantly close the terminal window, prevent that
#ifdef DNET_PLATFORM_WINDOWS
  DLOG_RAW("enter any key to exit\n> ");
  char f;
  std::cin >> f;
#endif

  return 0;
}
