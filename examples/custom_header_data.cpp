// ============================================================ //
// Headers
// ============================================================ //

#include "dnet/connection.hpp"
#include "dnet/header/packet_header.hpp"
#include "dnet/transport/tcp.hpp"
#include "dnet/util/util.hpp"
#include "dnet/util/platform.hpp"
#include "fmt/format.h"
#include "argparse.h"
#include <thread>
#include <string>
#include <cstdlib>
#include <iostream>
#include <mutex>

// ============================================================ //
// Print to console, real nice
// ============================================================ //

#define cprintln(...) dprintln("client", __VA_ARGS__)
#define sprintln(...) dprintln("server", __VA_ARGS__)
#define cprint(...) dprintln("client", __VA_ARGS__)
#define sprint(...) dprintln("server", __VA_ARGS__)

// print with fmt, prefixed with both function name and line number.
#define dprint(id, ...) tprint.print((id), __VA_ARGS__)
#define dprintln(id, ...) tprint.println((id), __VA_ARGS__)
#define dprintclean(...) tprint.printclean(__VA_ARGS__)

// sorry for template, it's helpful though
class Print_thread_safe
{
 private:
  std::mutex m_mutex{};

 public:
  template <typename ... TArgs>
  void print(const std::string& id, TArgs... args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    fmt::print("[{}] ", (id));
    aux(args...);
  }

  template <typename ... TArgs>
  void println(const std::string& id, TArgs... args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    fmt::print("[{}] ", (id));
    aux(args...);
    fmt::print("\n");
  }

  template <typename ... TArgs>
  void printclean(TArgs... args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    aux(args...);
  }

 private:
  template <typename T>
  void aux(T t) {
    fmt::print(t);;
  }

  template <typename T, typename ... TArgs>
  void aux(T t, TArgs... args) {
    fmt::print(t, args...);
  }

} tprint{};

// ============================================================ //
// Define our custom header data
// ============================================================ //

enum class Packet_type : u8
{
  kHandshake = 0,
  kPing,
  kPong,
  kShutdown
};

struct Custom_header_data
{
  Packet_type type;
  u32 id;
};


using CustomConnection = dnet::Connection<dnet::Tcp, Custom_header_data>;

std::string packet_type_to_string(Packet_type type) {
  std::string str;
  switch (type) {
    case Packet_type::kHandshake: {
      str = "Handshake";
      break;
    }
    case Packet_type::kPing: {
      str = "Ping";
      break;
    }
    case Packet_type::kPong: {
      str = "Pong";
      break;
    }
    case Packet_type::kShutdown: {
      str = "Shutdown";
      break;
    }
  }
  return str;
}

std::string header_data_to_string(Custom_header_data header) {
  std::string str{"type:"};
  str += packet_type_to_string(header.type);
  str += ",id:";
  str += std::to_string(header.id);
  return str;
}

// ============================================================ //
// Helper function
// ============================================================ //

// hepler function to crash on result fail
void die_on_fail(dnet::Result res, CustomConnection& con) {
  if (res == dnet::Result::kFail) {
    dprint("die_on_fail", "failed with error [{}]\n", con.last_error_to_string());
    std::cout << std::endl;  // flush
    exit(0);
  }
}

// ============================================================ //
// Client code
// ============================================================ //

void client(u16 port, const char* ip) {

  //connect to the server
  cprintln("connecting to {}:{}", ip, port);
  CustomConnection client{};
  dnet::Result res = client.connect(std::string(ip), port);
  die_on_fail(res, client);
  cprintln("connected");

  // sent handshake with empty payload
  {
    dnet::payload_container payload{};
    Custom_header_data header_data{};
    header_data.type = Packet_type::kHandshake;
    header_data.id = 1337; // why not

    // send the data
    cprintln("writing handshake");
    res = client.write(payload, header_data);
    die_on_fail(res, client);
    cprintln("wrote [{}|]", header_data_to_string(header_data));
  }

  // send echo request - ping
  {
    // prepare payload and header
    std::string msg("Hey there, from the client.");
    dnet::payload_container payload{msg.begin(), msg.end()};
    Custom_header_data header_data{};
    header_data.type = Packet_type::kPing;
    header_data.id = 55;

    // send the data
    cprintln("writing data");
    res = client.write(payload, header_data);
    die_on_fail(res, client);
    cprintln("wrote [{}|{}]", header_data_to_string(header_data), msg);
  }

  // wait for echo response - pong
  {
    dnet::payload_container payload{};
    cprint("waiting for response\n");
    auto [read_res, header_data] = client.read(payload);
    die_on_fail(read_res, client);
    cprintln("read [{}|{}]", header_data_to_string(header_data),
             std::string(payload.begin(), payload.end()));
  }

  // send shutdown request to server
  {
    // prepare payload and header
    dnet::payload_container payload{};
    Custom_header_data header_data{};
    header_data.type = Packet_type::kShutdown;
    header_data.id = 0;

    // send the data
    cprintln("writing data");
    res = client.write(payload, header_data);
    die_on_fail(res, client);
    cprintln("wrote [{}|]", header_data_to_string(header_data));
  }

  client.disconnect();
}

// ============================================================ //
// Server code
// ============================================================ //

// when a new client connects, this function will be run on a new thread
void serve(CustomConnection client, bool& run_server)
{
  // get port
  const auto [got_peer, peer_ip, peer_port] = client.get_peer();
  if (got_peer != dnet::Result::kSuccess) {
    sprintln("failed to get peer address with error [{}]",
           client.last_error_to_string());
    return;
  }

  dnet::payload_container payload;
  bool run = true;

  // wait for handshake packet
  {
    sprintln("[port:{}] waiting for handshake", peer_port);
    auto [res, header_data] = client.read(payload);
    if (res == dnet::Result::kSuccess &&
        header_data.type == Packet_type::kHandshake) {
      sprintln("[port:{}] read [{}|{}] - handshake success", peer_port,
               header_data_to_string(header_data),
               std::string(payload.begin(), payload.end()));
    }
    else {
      sprintln("failed to read handshake, closing client");
      client.disconnect();
      run = false;
    }
  }


  while (run) {

    // wait for packet
    sprintln("[port:{}] waiting for packet", peer_port);
    auto [res, header_data] = client.read(payload);
    if (res == dnet::Result::kSuccess) {

      // print packet
      sprintln("[port:{}] read [{}|{}]", peer_port,
               header_data_to_string(header_data),
               std::string(payload.begin(), payload.end()));

      // respond depending on packet type
      switch (header_data.type) {
        case Packet_type::kHandshake: {
          sprintln("[port:{}] handshake noop at this point", peer_port);
          break;
        }
        case Packet_type::kPing: {
          Custom_header_data header_data_pong{};
          header_data_pong.type = Packet_type::kPong;
          header_data_pong.id = 0;

          sprintln("[port:{}] echoing back the message", peer_port);
          res = client.write(payload, header_data_pong);
          if (res != dnet::Result::kSuccess) {
            sprint("[port:{}] failed to write with error [{}]\n",
                   peer_port, client.last_error_to_string());
            run = false;
          }
          break;
        }
        case Packet_type::kPong: {
          sprintln("[port:{}] pong not implemented", peer_port);
          break;
        }
        case Packet_type::kShutdown: {
          run_server = false;
          run = false;
          client.disconnect();
          break;
        }
      }


    }
    else {
      sprint("[port:{}] failed to read with error [{}]\n",
             peer_port, client.last_error_to_string());
      run = false;
    }
  }
}

// listen for connections on main thread
void run_server(u16 port)
{
  // start the server
  sprint("starting server\n");
  CustomConnection server{};
  dnet::Result res = server.start_server(port);
  die_on_fail(res, server);
  sprint("server running @ {}:{}\n", server.get_ip().value_or("error"),
         server.get_port().value_or(0));

  sprint("waiting for client\n");
  bool run = true;
  while (run) {

    // wait for client to connect
    if (server.can_accept()) {
      auto maybe_client = server.accept();
      if (maybe_client.has_value()) {
        const auto [got_peer, peer_ip, peer_port] = maybe_client.value().get_peer();
        if (got_peer == dnet::Result::kSuccess) {
          sprint("new client from {}:{}\n", peer_ip, peer_port);

          // handle the client on a seperate thread
          std::thread t(serve, std::move(maybe_client.value()), std::ref(run));
          t.detach();
        }
        else {
          sprint("failed to get peer with error [{}]\n",
                 server.last_error_to_string());
        }
      }
      else {
        sprint("failed to accept client with error [{}]\n",
               server.last_error_to_string());
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
      OPT_INTEGER('p', "port", &port, "port", NULL, NULL, 0),
      OPT_END(),
  };

  argparse argparse;
  argparse_init(&argparse, options, NULL, 0);
  argparse_describe(&argparse, NULL, NULL);
  argc = argparse_parse(&argparse, argc, argv);

  if (!ip) ip = "127.0.0.1";

  if (port < std::numeric_limits<u16>::min() ||
      port > std::numeric_limits<u16>::max()) {
    dprint("main", "invalid port provided, must be in the range [{}-{}]\n",
           std::numeric_limits<u16>::min(), std::numeric_limits<u16>::max());
    return 1;
  }

  if (!port) port = 1337;

  dnet::startup();
  std::thread server_thread(run_server, static_cast<u16>(port));

  cprintln("sleeping for 10 ms to let server start");
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  client(static_cast<u16>(port), "127.0.0.1");

  server_thread.join();

  dprintln("main", "sever and client closed successfully");

  // windows will instantly close the terminal window, prevent that
#ifdef DNET_PLATFORM_WINDOWS
  fmt::print("enter any key to exit\n> ");
  char f;
  std::cin >> f;
#endif

  dnet::shutdown();

  return 0;
}
