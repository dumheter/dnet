// ============================================================ //
// Headers
// ============================================================ //

#include <benchmark/benchmark.h>
#include <thirdparty/json/single_include/nlohmann/json.hpp>
#include <net/util/types.hpp>
#include <fmt/format.h>
#include <net/connection/connection.hpp>
#include <net/transport/tcp.hpp>
#include <net/header/packet_header.hpp>
#include <thread>

using namespace dnet;

// ====================================================================== //
// echo server
// ====================================================================== //

void serve(std::unique_ptr<Connection<Tcp, Packet_header>>&& client)
{
  try {
    payload_container payload;
    bool run = true;
    while (run) {
      client->read(payload);
      client->write(payload);
    }
  }
  catch (const dnet_exception&) {}
}

void run_echo_server(u16 port, bool run_once = false)
{
  try {
    Connection<Tcp, Packet_header> server{};
    server.start_server(port);

    bool run = true;
    do {
      auto client = server.accept();
      auto cli_ptr = std::make_unique<Connection<Tcp, Packet_header>>(std::move(client));
      std::thread t(serve, std::move(cli_ptr));
      t.detach();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (run && !run_once);
  }
  catch (const dnet_exception& e) {
    fmt::print("[echo_server] ex:{}\n", e.what());
  }
}

// ============================================================ //

Connection<Tcp, Packet_header> setup_connection(u16 port)
  {
    Connection<Tcp, Packet_header> con{};
    bool connected = false;
    while (!connected) {
      try {
        con.connect("127.0.0.1", port);
        connected = true;
      }
      catch (const dnet_exception&) {}
    }

    return con;
  }

// ====================================================================== //
// benchmark
// ====================================================================== //

constexpr int REP = 10;

static void BM_cbor_small(benchmark::State& state)
{
  auto con = setup_connection(1337);

  nlohmann::json j = {
    {"small sample",  "is small"},
    {"so very small", 1234123}
  };

  for (auto _ : state) {
    payload_container pl{};
    nlohmann::json::to_cbor(j, pl);
    try {
      con.write(pl);
    }
    catch (const dnet_exception& e) {
      fmt::print("[ex:{}]\n", e.what());
      break;
    }
  }
}
BENCHMARK(BM_cbor_small)->Repetitions(REP);

// ============================================================ //

static void BM_msgpack_small(benchmark::State& state)
{
  auto con = setup_connection(1337);

  nlohmann::json j = {
    {"small sample",  "is small"},
    {"so very small", 1234123}
  };

  for (auto _ : state) {
    payload_container pl{};
    nlohmann::json::to_msgpack(j, pl);
    try {
      con.write(pl);
    }
    catch (const dnet_exception& e) {
      fmt::print("[ex:{}]\n", e.what());
      break;
    }
  }
}
BENCHMARK(BM_msgpack_small)->Repetitions(REP);

// ============================================================ //

static void BM_ubjson_small(benchmark::State& state)
{
  auto con = setup_connection(1337);

  nlohmann::json j = {
    {"small sample",  "is small"},
    {"so very small", 1234123}
  };

  for (auto _ : state) {
    payload_container pl{};
    nlohmann::json::to_ubjson(j, pl);
    try {
      con.write(pl);
    }
    catch (const dnet_exception& e) {
      fmt::print("[ex:{}]\n", e.what());
      break;
    }
  }
}
BENCHMARK(BM_ubjson_small)->Repetitions(REP);

// ============================================================ //

BENCHMARK_MAIN();
