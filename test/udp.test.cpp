#include <doctest.h>
#include <dlog/dlog.hpp>
#include <dnet/net/udp.hpp>
#include <dnet/util/types.hpp>
#include <dutil/stopwatch.hpp>
#include <thread>
#include <vector>
#include <limits>

/**
 * Make the server stop by setting run == false or sending the 0xF packet.
 *
 * @param packets_out How many packets we received.
 */
void RunServer(const u16 port, bool& run, int& packets_out) {
  packets_out = 0;

  dnet::Udp con{};
  dnet::Result res = con.StartServer(port);
  CHECK(res == dnet::Result::kSuccess);
  if (res != dnet::Result::kSuccess) {
    DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
  }

  std::vector<u8> payload{};
  payload.reserve(std::numeric_limits<u16>::max());

  while (run && res == dnet::Result::kSuccess) {

    if (con.CanRead()) {
      payload.resize(payload.capacity());
      auto maybe_bytes = con.Read(payload.data(), payload.capacity());
      CHECK(maybe_bytes.has_value());
      if (maybe_bytes.has_value()) {
        payload.resize(maybe_bytes.value());

        ++packets_out;
        std::string msg{payload.begin(), payload.end()};
        if (payload.size() == 1 && payload[0] == 0xF) {
          run = false;
        }
      }
      else {
        run = false;
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
    dnet::Udp con{};

    dnet::Result res = dnet::Result::kFail;
    while (run && res != dnet::Result::kSuccess) {
      res = con.Connect("localhost", port);
    }
    CHECK(res == dnet::Result::kSuccess);

    int packets = 0;
    if (run) {
      // send a one packet
      {
        const std::string str{"hey from client"};
        std::vector<u8> payload{str.begin(), str.end()};
        auto maybe_bytes = con.Write(payload.data(), payload.size());
        CHECK(maybe_bytes.has_value());
        if (maybe_bytes.has_value()) {
          CHECK(maybe_bytes.value() == payload.size());
          ++packets;
        }
      }

      // send a second packet
      {
        const std::string str{"My second message!"};
        std::vector<u8> payload{str.begin(), str.end()};
        auto maybe_bytes = con.Write(payload.data(), payload.size());
        CHECK(maybe_bytes.has_value());
        if (maybe_bytes.has_value()) {
          CHECK(maybe_bytes.value() == payload.size());
          ++packets;
        }
      }

      // send a shutdown packet
      {
        std::vector<u8> payload{};
        payload.push_back(0xF);
        auto maybe_bytes = con.Write(payload.data(), payload.size());
        CHECK(maybe_bytes.has_value());
        if (maybe_bytes.has_value()) {
          CHECK(maybe_bytes.value() == payload.size());
          ++packets;
        }
      }
    }

    CHECK(packets == 3);

    if (res != dnet::Result::kSuccess) {
      DLOG_INFO("chif_net error: {}", con.LastErrorToString());
    }

    run = false;
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
    dnet::Udp con{};

    dnet::Result res = dnet::Result::kFail;
    while (run && res != dnet::Result::kSuccess) {
      res = con.Connect("localhost", port);
    }

    int packets = 0;
    if (run) {
      std::vector<u8> payload{};

      // send first packet
      {
        constexpr int max_udp_packet_size = 65507;
        for (int i = 0; i < max_udp_packet_size; i++) {
          const u8 val = i % std::numeric_limits<u8>::max();
          payload.push_back(val);
        }
        const auto maybe_bytes = con.Write(payload.data(), payload.size());
        CHECK(maybe_bytes.has_value());
        if (maybe_bytes.has_value()) {
          CHECK(maybe_bytes.value() == payload.size());
          ++packets;
        }
        else {
          DLOG_WARNING("failed to write [{}]", con.LastErrorToString());
        }
      }

      {
        // fail to send second, too big, packet
        payload.push_back(1);
        const auto maybe_bytes = con.Write(payload.data(), payload.size());
        CHECK(!maybe_bytes.has_value());
      }

      // send a shutdown packet
      {
        std::vector<u8> end_payload{};
        end_payload.push_back(0xF);
        auto maybe_bytes = con.Write(end_payload.data(), end_payload.size());
        CHECK(maybe_bytes.has_value());
        if (maybe_bytes.has_value()) {
          CHECK(maybe_bytes.value() == end_payload.size());
          ++packets;
        } else {
          DLOG_WARNING("failed to write [{}]", con.LastErrorToString());
        }
      }
    }

    CHECK(packets == 2);

    CHECK(res == dnet::Result::kSuccess);
    if (res != dnet::Result::kSuccess) {
      DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
    }

    run = false;
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

  CHECK(packets == 2);
}
