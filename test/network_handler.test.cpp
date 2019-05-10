#include <doctest.h>
#include <dlog/dlog.hpp>
#include <dnet/net/udp.hpp>
#include <dnet/network_handler.hpp>
#include <dnet/util/types.hpp>
#include <dutil/stopwatch.hpp>
#include <functional>
#include <limits>
#include <thread>
#include <vector>

/**
 * Make the server stop by setting run == false or sending the 0xF packet.
 *
 * @param packets_out How many packets we received.
 */
static void RunEchoServer(const u16 port, bool& run, int& packets_out) {
  packets_out = 0;

  dnet::Udp con{};
  dnet::Result res = con.StartServer(port);
  CHECK(res == dnet::Result::kSuccess);
  if (res != dnet::Result::kSuccess) {
    DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
  }

  std::vector<u8> payload(std::numeric_limits<u16>::max());
  std::string src_addr;
  u16 src_port;

  while (run && res == dnet::Result::kSuccess) {
    if (con.CanRead()) {
      payload.resize(payload.capacity());

      auto maybe_bytes =
          con.ReadFrom(payload.data(), payload.size(), src_addr, src_port);
      CHECK(maybe_bytes.has_value());
      if (maybe_bytes.has_value()) {
        payload.resize(maybe_bytes.value());

        ++packets_out;
        std::string msg{payload.begin(), payload.end()};
        if (payload.size() == 1 && payload[0] == 0xF) {
          run = false;
        } else {
          auto maybe_bytes =
              con.WriteTo(payload.data(), payload.size(), src_addr, src_port);
          if (!maybe_bytes.has_value() ||
              maybe_bytes.value() != static_cast<ssize_t>(payload.size())) {
            DLOG_WARNING("failed to write, could not echo back. [{}]",
                         con.LastErrorToString());
            CHECK(!maybe_bytes.has_value());
            if (maybe_bytes.has_value()) {
              CHECK(maybe_bytes.value() !=
                    static_cast<ssize_t>(payload.size()));
            }
          }
        }
      } else {
        run = false;
      }
    }
  }

  if (res != dnet::Result::kSuccess) {
    DLOG_WARNING("Result::kFail [{}]", con.LastErrorToString());
  }

  // ensure this is false
  run = false;
}

// ============================================================ //

TEST_CASE("network handler basics") {
  const auto RunClient = [](const u16 port, bool& run) {
    dnet::NetworkHandler<std::vector<u8>, dnet::Udp> nh{};
    dutil::Stopwatch sw{};
    const auto WaitForEvent = [&]() -> bool {
      std::function<bool()> fn = std::bind(
          &dnet::NetworkHandler<std::vector<u8>, dnet::Udp>::HasEvent, &nh);
      constexpr s64 timeout_ms = 200;
      const bool check = dutil::TimedCheck(fn, timeout_ms);
      return check;
    };

    {  // connect & get the connected event
      sw.Start();
      nh.Connect("localhost", port);

      REQUIRE(WaitForEvent());
      sw.Stop();
      auto event = nh.GetEvent();
      REQUIRE(event.type() == dnet::NetworkEvent::Type::kConnected);
      DLOG_VERBOSE("connecting took {:.2f} ms", sw.fms());
    }

    {  // send a message and get it back
      std::string msg{"this is a message"};
      std::vector<u8> packet{msg.begin(), msg.end()};
      sw.Start();
      CHECK(!nh.HasEvent());
      REQUIRE(nh.Send(packet));

      REQUIRE(WaitForEvent());
      auto event = nh.GetEvent();
      REQUIRE(event.type() == dnet::NetworkEvent::Type::kNewData);

      auto maybe_packet = nh.Recv();
      sw.Stop();
      REQUIRE(maybe_packet.has_value());
      packet = std::move(maybe_packet.value());
      std::string echo_msg{packet.begin(), packet.end()};
      CHECK(echo_msg == msg);
      DLOG_VERBOSE("time from sending, to receiving echo, was {:.2f} ms",
                   sw.fms());
    }

    {  // send 3 60kb packets and get them bask
      constexpr u32 size = 60000;
      std::vector<u8> packet0(size);
      for (u32 i = 0; i < size; i++) {
        packet0[i] = static_cast<u8>(i % 256);
      }
      sw.Start();
      CHECK(!nh.HasEvent());
      REQUIRE(nh.Send(packet0));
      REQUIRE(nh.Send(packet0));
      REQUIRE(nh.Send(packet0));

      for (int i = 0; i < 3; i++) {
        REQUIRE(WaitForEvent());
        auto event = nh.GetEvent();
        REQUIRE(event.type() == dnet::NetworkEvent::Type::kNewData);

        auto maybe_packet = nh.Recv();

        REQUIRE(maybe_packet.has_value());
        std::vector<u8> packet1 = std::move(maybe_packet.value());
        REQUIRE(packet0.size() == packet1.size());
        CHECK(std::memcmp(packet0.data(), packet1.data(), packet0.size()) == 0);
      }
      sw.Stop();
      DLOG_VERBOSE(
          "sending, receiving and checking three 60Kb packets took {:.2f} ms",
          sw.fms());
    }

    {  // send disconnect packet
      std::vector<u8> packet(1);
      packet[0] = 0xF;
      const auto did_enqueue = nh.Send(packet);
      CHECK(did_enqueue);
    }

    // allow for the worker thread to send the last packet
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    run = false;
  };

  constexpr u16 port = 2049;
  bool run_server = true;
  int packets = 0;
  std::thread server_thread{RunEchoServer, port, std::ref(run_server),
                            std::ref(packets)};
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

  CHECK(packets == 5);
}
