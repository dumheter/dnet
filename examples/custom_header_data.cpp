#include <argparse.h>
#include <cstdlib>
#include <dlog.hpp>
#include <dutil/types.hpp>
#include <dnet/net/packet_header.hpp>
#include <dnet/tcp_connection.hpp>
#include <dnet/util/platform.hpp>
#include <dnet/util/util.hpp>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// ============================================================ //
// Define our custom header data
// ============================================================ //

enum class PacketType : u8 { kHandshake = 0, kPing, kPong, kShutdown };

struct CustomHeaderData {
  PacketType type;
  u32 id;
};

using CustomConnection = dnet::TcpConnection<std::vector<u8>, CustomHeaderData>;

std::string PacketTypeToString(const PacketType type) {
  std::string str;
  switch (type) {
    case PacketType::kHandshake: {
      str = "Handshake";
      break;
    }
    case PacketType::kPing: {
      str = "Ping";
      break;
    }
    case PacketType::kPong: {
      str = "Pong";
      break;
    }
    case PacketType::kShutdown: {
      str = "Shutdown";
      break;
    }
  }
  return str;
}

std::string HeaderDataToString(const CustomHeaderData header) {
  std::string str{"type:"};
  str += PacketTypeToString(header.type);
  str += ",id:";
  str += std::to_string(header.id);
  return str;
}

// ============================================================ //
// Helper function
// ============================================================ //

// hepler function to crash on result fail
void DieOnFail(const dnet::Result res, CustomConnection& con) {
  if (res == dnet::Result::kFail) {
    DLOG_INFO("die_on_fail", "failed with error [{}]",
                con.LastErrorToString());
    dlog::Flush();
    exit(0);
  }
}

// ============================================================ //
// Client code
// ============================================================ //

void Client(const u16 port, const char* ip) {
  // connect to the server
  DLOG_INFO("client", "connecting to {}:{}", ip, port);
  CustomConnection client{};
  dnet::Result res = client.Connect(std::string(ip), port);
  DieOnFail(res, client);
  DLOG_INFO("client", "connected");

  // sent handshake with empty payload
  {
    std::vector<u8> payload{};
    CustomHeaderData header_data{};
    header_data.type = PacketType::kHandshake;
    header_data.id = 1337;  // why not

    // send the data
    DLOG_INFO("client", "writing handshake");
    res = client.Write(header_data, payload);
    DieOnFail(res, client);
    DLOG_INFO("client", "wrote [{}|]", HeaderDataToString(header_data));
  }

  // send echo request - ping
  {
    // prepare payload and header
    std::string msg("Hey there, from the client.");
    std::vector<u8> payload{msg.begin(), msg.end()};
    CustomHeaderData header_data{};
    header_data.type = PacketType::kPing;
    header_data.id = 55;

    // send the data
    DLOG_INFO("client", "writing data");
    res = client.Write(header_data, payload);
    DieOnFail(res, client);
    DLOG_INFO("client", "wrote [{}|{}]", HeaderDataToString(header_data),
                msg);
  }

  // wait for echo response - pong
  {
    std::vector<u8> payload{};
    DLOG_INFO("client", "waiting for response");
    auto [read_res, header_data] = client.Read(payload);
    DieOnFail(read_res, client);
    DLOG_INFO("client", "read [{}|{}]", HeaderDataToString(header_data),
                std::string(payload.begin(), payload.end()));
  }

  // send shutdown request to server
  {
    // prepare payload and header
    std::vector<u8> payload{};
    CustomHeaderData header_data{};
    header_data.type = PacketType::kShutdown;
    header_data.id = 0;

    // send the data
    DLOG_INFO("client", "writing data");
    res = client.Write(header_data, payload);
    DieOnFail(res, client);
    DLOG_INFO("client", "wrote [{}|]", HeaderDataToString(header_data));
  }

  client.Disconnect();
}

// ============================================================ //
// Server code
// ============================================================ //

// when a new client connects, this function will be run on a new thread
void Serve(CustomConnection client, bool& run_server) {
  // get port
  const auto [got_peer, peer_ip, peer_port] = client.GetPeer();
  if (got_peer != dnet::Result::kSuccess) {
    DLOG_ERROR("serve", "failed to get peer address with error [{}]",
                 client.LastErrorToString());
    return;
  }

  std::vector<u8> payload;
  bool run = true;

  // wait for handshake packet
  {
    DLOG_INFO("serve", "[port:{}] waiting for handshake", peer_port);
    auto [res, header_data] = client.Read(payload);
    if (res == dnet::Result::kSuccess &&
        header_data.type == PacketType::kHandshake) {
      DLOG_INFO("client", "[port:{}] read [{}|{}] - handshake success",
                  peer_port, HeaderDataToString(header_data),
                  std::string(payload.begin(), payload.end()));
    } else {
      DLOG_WARNING("serve", "failed to read handshake, closing client");
      client.Disconnect();
      run = false;
    }
  }

  while (run) {
    // wait for packet
    DLOG_INFO("serve", "[port:{}] waiting for packet", peer_port);
    auto [res, header_data] = client.Read(payload);
    if (res == dnet::Result::kSuccess) {
      // print packet
      DLOG_INFO("serve", "[port:{}] read [{}|{}]", peer_port,
                  HeaderDataToString(header_data),
                  std::string(payload.begin(), payload.end()));

      // respond depending on packet type
      switch (header_data.type) {
        case PacketType::kHandshake: {
          DLOG_INFO("serve", "[port:{}] handshake noop at this point",
                      peer_port);
          break;
        }
        case PacketType::kPing: {
          CustomHeaderData header_data_pong{};
          header_data_pong.type = PacketType::kPong;
          header_data_pong.id = 0;

          DLOG_INFO("serve", "[port:{}] echoing back the message", peer_port);
          res = client.Write(header_data_pong, payload);
          if (res != dnet::Result::kSuccess) {
            DLOG_INFO("serve", "[port:{}] failed to write with error [{}]",
                        peer_port, client.LastErrorToString());
            run = false;
          }
          break;
        }
        case PacketType::kPong: {
          DLOG_INFO("serve", "[port:{}] pong not implemented", peer_port);
          break;
        }
        case PacketType::kShutdown: {
          run_server = false;
          run = false;
          client.Disconnect();
          break;
        }
      }

    } else {
      DLOG_WARNING("server", "[port:{}] failed to read with error [{}]\n",
                     peer_port, client.LastErrorToString());
      run = false;
    }
  }
}

// listen for connections on main thread
void RunServer(const u16 port) {
  // start the server
  DLOG_INFO("server", "starting server");
  CustomConnection server{};
  dnet::Result res = server.StartServer(port);
  DieOnFail(res, server);
  DLOG_INFO("server", "server running @ {}:{}\n",
              server.GetIp().value_or("error"), server.GetPort().value_or(0));

  DLOG_INFO("server", "waiting for client\n");
  bool run = true;
  while (run) {
    // wait for client to connect
    if (server.CanAccept()) {
      auto maybe_client = server.Accept();
      if (maybe_client.has_value()) {
        const auto [got_peer, peer_ip, peer_port] =
            maybe_client.value().GetPeer();
        if (got_peer == dnet::Result::kSuccess) {
          DLOG_INFO("server", "new client from {}:{}\n", peer_ip, peer_port);

          // handle the client on a seperate thread
          std::thread t(Serve, std::move(maybe_client.value()), std::ref(run));
          t.detach();
        } else {
          DLOG_INFO("server", "failed to get peer with error [{}]\n",
                      server.LastErrorToString());
        }
      } else {
        DLOG_INFO("server", "failed to accept client with error [{}]\n",
                    server.LastErrorToString());
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

// ============================================================ //
// Main
// ============================================================ //

int main(int argc, const char** argv) {
  int port = 0;
  const char* ip = NULL;
  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_GROUP("Settings"),
      OPT_INTEGER('p', "port", &port, "port", NULL, 0, 0),
      OPT_END(),
  };

  argparse argparse;
  argparse_init(&argparse, options, NULL, 0);
  argparse_describe(&argparse, NULL, NULL);
  argc = argparse_parse(&argparse, argc, argv);

  if (!ip) ip = "127.0.0.1";

  if (port < std::numeric_limits<u16>::min() ||
      port > std::numeric_limits<u16>::max()) {
    DLOG_INFO("main", "invalid port provided, must be in the range [{}-{}]",
                std::numeric_limits<u16>::min(),
                std::numeric_limits<u16>::max());
    return 1;
  }

  if (!port) port = 1337;

  dnet::Startup();
  std::thread server_thread(RunServer, static_cast<u16>(port));

  DLOG_INFO("main", "sleeping for 10 ms to let server start");
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  Client(static_cast<u16>(port), "127.0.0.1");

  server_thread.join();

  DLOG_INFO("main", "sever and client closed successfully");

  // windows will instantly close the terminal window, prevent that
#ifdef DNET_PLATFORM_WINDOWS
  DLOG_RAW("enter any key to exit\n> ");
  char f;
  std::cin >> f;
#endif

  dnet::Shutdown();

  return 0;
}
