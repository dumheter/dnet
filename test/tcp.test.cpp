#include <doctest.h>
#include <dlog/dlog.hpp>
#include <dnet/connection.hpp>
#include <dnet/net/tcp.hpp>
#include <dnet/util/types.hpp>
#include <dutil/stopwatch.hpp>
#include <memory>
#include <thread>
#include <vector>

struct TestHeaderData {
  const u8 magic_number = 14;
};

using TestConnection =
    dnet::Connection<std::vector<u8>, dnet::Tcp, TestHeaderData>;

// ============================================================ //
// Basic tcp test - send 300 packets
// ============================================================ //

static void RunServer(const u16 port, bool& run) {
  TestConnection con{};
  auto res = con.StartServer(port);

  // run server and accept one client
  std::unique_ptr<TestConnection> client;
  bool has_client = false;
  while (run && !has_client) {
    if (con.CanAccept()) {
      auto maybe_con = con.Accept();
      CHECK(maybe_con.has_value() == true);
      if (maybe_con.has_value()) {
        client = std::make_unique<TestConnection>(std::move(maybe_con.value()));
        has_client = true;
      } else {
        run = false;
      }
    }
  }
  CHECK(res == dnet::Result::kSuccess);
  if (res != dnet::Result::kSuccess) {
    DLOG_WARNING("last error {}", con.LastErrorToString());
  }

  // run the client
  std::vector<u8> payload{};
  payload.reserve(2048);
  int packets = 0;
  while (run && res == dnet::Result::kSuccess) {
    if (client->CanRead()) {
      auto [rres, header_data] = client->Read(payload);
      if (rres == dnet::Result::kSuccess) {
        ++packets;
      }
    }
    if (packets == 30300) {
      run = false;
    }
  }
  CHECK(packets == 30300);
  CHECK(res == dnet::Result::kSuccess);
  if (res != dnet::Result::kSuccess) {
    DLOG_WARNING("last error {}", client->LastErrorToString());
  }
}

static void RunClient(const u16 port, bool& run) {
  TestConnection con{};

  dnet::Result res = dnet::Result::kFail;
  while (run && res != dnet::Result::kSuccess) {
    res = con.Connect("localhost", port);
  }

  int packets = 30300;
  while (run && res == dnet::Result::kSuccess) {
    if (packets-- > 0) {
      const std::string msg{"this is a message that I am sending"};
      std::vector<u8> payload{msg.begin(), msg.end()};
      TestHeaderData header_data{};
      res = con.Write(header_data, payload);
    } else {
      run = false;
    }
  }

  CHECK(res == dnet::Result::kSuccess);
  if (res != dnet::Result::kSuccess) {
    DLOG_WARNING("last error {}", con.LastErrorToString());
  }
}

TEST_CASE("tcp basic") {
  constexpr u16 port = 12021;
  bool run_server = true;
  std::thread server_thread{RunServer, port, std::ref(run_server)};
  bool run_client = true;
  std::thread client_thread{RunClient, port, std::ref(run_client)};

  dutil::Stopwatch stopwatch{};
  stopwatch.start();
  constexpr long timeout_ms = 2000;
  bool did_timeout = false;
  while (run_server || run_client) {
    if (stopwatch.now_ms() > timeout_ms) {
      did_timeout = true;
      break;
    }
  }
  CHECK(did_timeout == false);

  run_server = false;
  run_client = false;
  server_thread.join();
  client_thread.join();
}

// ============================================================ //
