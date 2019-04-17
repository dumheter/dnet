#include "doctest.h"
#include <dnet/connection.hpp>
#include <dutil/stopwatch.hpp>
#include <dlog/dlog.hpp>
#include <thread>

// ============================================================ //
// Setup
// ============================================================ //

enum class PacketType : u8 {
  kOne,
  kTwo,
  kThree
};

/**
 * You make up your own header data
 */
struct TestHeaderData {
  PacketType type;
  const u8 version = 8;
  u32 some_number;
};

std::string PacketTypeToString(PacketType type) {
  std::string str{};
  switch (type) {
    case PacketType::kOne:
      str += "One";
      break;
    case PacketType::kTwo:
      str += "Two";
      break;
    case PacketType::kThree:
      str += "Three";
      break;
  }
  return str;
}

std::string HeaderDataToString(const TestHeaderData& header_data) {
  std::string str{"{type:"};
  str += PacketTypeToString(header_data.type);
  str += ", version:" + std::to_string(header_data.version);
  str += ", some_number:" + std::to_string(header_data.some_number);
  str += "}";
  return str;
}

using TestConnection = dnet::Connection<dnet::Udp, TestHeaderData>;

// ============================================================ //

void RunServer(const u16 port, bool& run) {
  TestConnection con{};
  dnet::Result res = con.start_server(port);
  CHECK(res == dnet::Result::kSuccess);

  dnet::payload_container payload{};
  payload.reserve(2048);

  int packets = 0;

  while (run && res == dnet::Result::kSuccess) {

    if (con.can_read()) {
      const auto [read_res, header_data] = con.read(payload);
      CHECK(read_res == dnet::Result::kSuccess);
      res = read_res;
      if (read_res == dnet::Result::kSuccess) {
        ++packets;
        std::string msg{payload.begin(), payload.end()};
        if (header_data.type == PacketType::kThree) {
          run = false;
        }
      }
    }
  }

  if (res != dnet::Result::kSuccess) {
    DLOG_INFO("chif_net error: {}", con.last_error_to_string());
  }

  CHECK(packets == 3);
}

void RunClient(const u16 port, bool& run) {
  // allow for the server to start
  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  TestConnection con{};
  dnet::Result res = con.connect("localhost", port);
  CHECK(res == dnet::Result::kSuccess);

  // send a one packet
  {
    const std::string str{"hey from client"};
    dnet::payload_container payload{str.begin(), str.end()};
    TestHeaderData header_data{};
    header_data.type = PacketType::kOne;
    header_data.some_number = 5;
    res = con.write(payload, header_data);
    CHECK(res == dnet::Result::kSuccess);
  }

  // send a two packet
  {
    const std::string str{"second packet"};
    dnet::payload_container payload{str.begin(), str.end()};
    TestHeaderData header_data{};
    header_data.type = PacketType::kTwo;
    header_data.some_number = 1337;
    res = con.write(payload, header_data);
    CHECK(res == dnet::Result::kSuccess);
  }

  // send a three packet
  {
    const std::string str{"last packet, you should stop running"};
    dnet::payload_container payload{str.begin(), str.end()};
    TestHeaderData header_data{};
    header_data.type = PacketType::kThree;
    header_data.some_number = 0;
    res = con.write(payload, header_data);
    CHECK(res == dnet::Result::kSuccess);
  }

  while (run && res == dnet::Result::kSuccess) {
    run = false;
  }

  if (res != dnet::Result::kSuccess) {
    DLOG_INFO("chif_net error: {}", con.last_error_to_string());
  }
}

// ============================================================ //
// Tests
// ============================================================ //

TEST_CASE("udp basics") {
  constexpr u16 port = 2048;
  bool run_server = true;
  std::thread server_thread{RunServer, port, std::ref(run_server)};
  bool run_client = true;
  std::thread client_thread{RunClient, port, std::ref(run_client)};

  dutil::Stopwatch stopwatch{};
  stopwatch.start();
  constexpr long timeout_ms = 5000;
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
