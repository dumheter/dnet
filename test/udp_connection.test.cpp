#include <doctest.h>
#include <dlog.hpp>
#include <dnet/udp_connection.hpp>
#include <dnet/util/types.hpp>
#include <dutil/stopwatch.hpp>
#include <memory>
#include <thread>
#include <vector>

// ============================================================ //
// Basic example
// ============================================================ //

static void RunServer(const u16 port, bool& run) {
  dnet::UdpBuffer<std::vector<u8>> buffer;
  dnet::UdpConnection<std::vector<u8>> con{};
  auto res = con.StartServer(port);
  if (res == dnet::Result::kFail) {
    DLOG_ERROR("failed");
  }

  run = false;
}

static void RunClient(const u16 port, bool& run) {
  dnet::UdpBuffer<std::vector<u8>> buffer;
  dnet::UdpConnection<std::vector<u8>> con{};
  auto res = con.Connect("localhost", port);
  if (res == dnet::Result::kFail) {
    DLOG_ERROR("failed");
  }

  // con.Write

  run = false;
}

TEST_CASE("udp basic") {
  constexpr u16 port = 3000;
  bool run_server = true;
  std::thread server_thread{RunServer, port, std::ref(run_server)};
  bool run_client = true;
  std::thread client_thread{RunClient, port, std::ref(run_client)};

  dutil::Stopwatch stopwatch{};
  stopwatch.Start();
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
