#include <doctest.h>
#include <dlog/dlog.hpp>
#include <dnet/net/udp.hpp>
#include <dnet/tcp_connection.hpp>
#include <dnet/util/types.hpp>
#include <dutil/stopwatch.hpp>
#include <thread>
#include <vector>

// ============================================================ //
// Setup
// ============================================================ //

enum class PacketType : u8 { kOne, kTwo, kThree };

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

using TestConnection =
    dnet::TcpConnection<std::vector<u8>, TestHeaderData>;

// ============================================================ //

void RunServer(const u16 port, bool& run, int& packets) {
  packets = 0;

  TestConnection con{};
  dnet::Result res = con.StartServer(port);
  CHECK(res == dnet::Result::kSuccess);
  if (res != dnet::Result::kSuccess) {
    DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
  }

  std::vector<u8> payload{};
  payload.reserve(2048);

  while (run && res == dnet::Result::kSuccess) {
    if (con.CanRead()) {
      auto [read_res, header_data] = con.Read(payload);
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
    DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
  }
}

// ============================================================ //

TEST_CASE("udp basics") {
  const auto RunClient = [](const u16 port, bool& run) {
    TestConnection con{};

    dnet::Result res = dnet::Result::kFail;
    while (run && res != dnet::Result::kSuccess) {
      res = con.Connect("localhost", port);
    }
    CHECK(res == dnet::Result::kSuccess);

    // send a one packet
    {
      const std::string str{"hey from client"};
      std::vector<u8> payload{str.begin(), str.end()};
      TestHeaderData header_data{};
      header_data.type = PacketType::kOne;
      header_data.some_number = 5;
      res = con.Write(header_data, payload);
      CHECK(res == dnet::Result::kSuccess);
    }

    // send a two packet
    {
      const std::string str{"second packet"};
      std::vector<u8> payload{str.begin(), str.end()};
      TestHeaderData header_data{};
      header_data.type = PacketType::kTwo;
      header_data.some_number = 1337;
      res = con.Write(header_data, payload);
      CHECK(res == dnet::Result::kSuccess);
    }

    // send a three packet
    {
      const std::string str{"last packet, you should stop running"};
      std::vector<u8> payload{str.begin(), str.end()};
      TestHeaderData header_data{};
      header_data.type = PacketType::kThree;
      header_data.some_number = 0;
      res = con.Write(header_data, payload);
      CHECK(res == dnet::Result::kSuccess);
    }

    while (run && res == dnet::Result::kSuccess) {
      run = false;
    }

    if (res != dnet::Result::kSuccess) {
      DLOG_INFO("chif_net error: {}", con.LastErrorToString());
    }
  };

  constexpr u16 port = 2048;
  bool run_server = true;
  int packets = 0;
  std::thread server_thread{RunServer, port, std::ref(run_server),
                            std::ref(packets)};
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

  CHECK(packets == 3);
}

// ============================================================ //

TEST_CASE("udp big packet") {
  const auto RunClient = [](const u16 port, bool& run) {
    TestConnection con{};

    dnet::Result res = dnet::Result::kFail;
    while (run && res != dnet::Result::kSuccess) {
      res = con.Connect("localhost", port);
    }
    CHECK(res == dnet::Result::kSuccess);
    if (res != dnet::Result::kSuccess) {
      DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
    }

    std::vector<u8> payload{};
    constexpr int max_udp_packet_size = 65507;
    for (int i = 0; i < max_udp_packet_size + 1; i++) {
      payload.push_back(1);
    }
    DLOG_INFO("size {}", payload.size());
    TestHeaderData header_data{};
    header_data.type = PacketType::kThree;
    res = con.Write(header_data, payload);
    CHECK(res == dnet::Result::kSuccess);
    if (res != dnet::Result::kSuccess) {
      DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
    }

    while (run && res == dnet::Result::kSuccess) {
      run = false;
    }

    CHECK(res == dnet::Result::kSuccess);
    if (res != dnet::Result::kSuccess) {
      DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  };

  constexpr u16 port = 2049;
  bool run_server = true;
  int packets = 0;
  std::thread server_thread{RunServer, port, std::ref(run_server),
                            std::ref(packets)};
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
