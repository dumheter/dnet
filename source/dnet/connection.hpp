#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include "dnet/header/packet_header.hpp"
#include "dnet/payload/payload.hpp"
#include "dnet/transport/tcp.hpp"
#include "dnet/util/dnet_assert.hpp"
#include "dnet/util/result.hpp"
#include "dnet/util/types.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet {

/**
 * TODO Make payload container be part of the template?
 * @tparam TTransport
 * @tparam THeaderData Provide your own data in the header! Note, packet size is
 * handled internally.
 */
template <typename TTransport = Tcp, typename THeaderData = Header_data_example>
class Connection {
 public:
  static_assert(std::is_standard_layout<THeaderData>::value,
                "THeaderData must be trivial (C struct) in order to serialize "
                "correctly.");

  using Header = Packet_header<THeaderData>;

  // ====================================================================== //
  // Lifetime
  // ====================================================================== //

  Connection();
  explicit Connection(TTransport&& transport);

  // no copy
  Connection(const Connection& other) = delete;
  Connection& operator=(const Connection& other) = delete;

  Connection(Connection&& other) noexcept;
  Connection& operator=(Connection&& other) noexcept;

  // ====================================================================== //
  // Client methods
  // ====================================================================== //

  Result connect(const std::string& ip, u16 port);

  void disconnect();

  std::tuple<Result, THeaderData> read(payload_container& payload_out);

  // std::optional<payload_container> read();

  // TODO implement this
  // void read(u8* payload, size_t payload_size);

  Result write(const payload_container& payload, THeaderData header_data);
  Result write(const u8* payload, typename Header::Payload_size payload_size,
               THeaderData header_data);

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool can_read() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool can_write() const;

  // ====================================================================== //
  // Server methods
  // ====================================================================== //

  Result start_server(u16 port);

  std::optional<Connection> accept();

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool can_accept() const;

  // ====================================================================== //
  // General methods
  // ====================================================================== //

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool has_error() const;

  std::optional<std::string> get_ip() { return m_transport.get_ip(); }

  std::optional<u16> get_port() { return m_transport.get_port(); }

  std::string last_error_to_string() const {
    return m_transport.last_error_to_string();
  }

  /**
   * Get the address information about the peer which we are connected to.
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> get_peer() {
    return m_transport.get_peer();
  }

  // ====================================================================== //
  // Data members
  // ====================================================================== //

 private:
  TTransport m_transport;
};

// ====================================================================== //
// template definition
// ====================================================================== //

template <typename TTransport, typename THeaderData>
Connection<TTransport, THeaderData>::Connection() : m_transport() {}

template <typename TTransport, typename THeaderData>
Connection<TTransport, THeaderData>::Connection(TTransport&& transport)
    : m_transport(std::move(transport)) {}

template <typename TTransport, typename THeaderData>
Connection<TTransport, THeaderData>::Connection(
    Connection<TTransport, THeaderData>&& other) noexcept
    : m_transport(std::move(other.m_transport)) {}

template <typename TTransport, typename THeaderData>
Connection<TTransport, THeaderData>& Connection<TTransport, THeaderData>::
operator=(Connection<TTransport, THeaderData>&& other) noexcept {
  if (&other != this) {
    m_transport = std::move(other.m_transport);
  }
  return *this;
}

template <typename TTransport, typename THeaderData>
Result Connection<TTransport, THeaderData>::connect(const std::string& ip,
                                                    u16 port) {
  return m_transport.connect(ip, port);
}

template <typename TTransport, typename THeaderData>
void Connection<TTransport, THeaderData>::disconnect() {
  m_transport.disconnect();
}

// TODO go over the types used
template <typename TTransport, typename THeaderData>
std::tuple<Result, THeaderData> Connection<TTransport, THeaderData>::read(
    payload_container& payload_out) {
  Header header{};
  ssize_t bytes = 0;
  // TODO make it possible to break out of loops if bad header
  while (bytes < Header::get_header_size()) {
    const auto maybe_bytes =
        m_transport.read(header.get(), Header::get_header_size());
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return std::make_tuple<Result, THeaderData>(Result::kFail, THeaderData{});
    }
  }

  if (payload_out.capacity() < header.get_payload_size()) {
    payload_out.reserve(header.get_payload_size());
  }

  payload_out.resize(payload_out.capacity());
  bytes = 0;
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.get_payload_size()) {
    // TODO timeout read in case bad info in header
    const auto maybe_bytes = m_transport.read(
        &payload_out[bytes], header.get_payload_size() - bytes);
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return std::make_tuple<Result, THeaderData>(Result::kFail,
                                                  header.get_header_data());
    }
  }
  payload_out.resize(static_cast<size_t>(bytes));
  return std::make_tuple<Result, THeaderData>(Result::kSuccess,
                                              header.get_header_data());
}

template <typename TTransport, typename THeaderData>
Result Connection<TTransport, THeaderData>::write(
    const payload_container& payload, THeaderData header_data) {
  const auto payload_size = payload.size();
  if (payload_size > std::numeric_limits<Header::Payload_size>::max() ||
      payload_size < std::numeric_limits<Header::Payload_size>::min()) {
    // TODO send payloads larger than what can fit in a single packet
    DNET_ASSERT(
        payload_size > std::numeric_limits<Header::Payload_size>::max() ||
            payload_size < std::numeric_limits<Header::Payload_size>::min(),
        "Cannot fit the payload in the packet");
  }
  const Header header{static_cast<Header::Payload_size>(payload_size),
                      header_data};

  ssize_t bytes = 0;
  // TODO make it possible to break out of loops if bad header
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.get_header_size()) {
    const auto maybe_bytes =
        m_transport.write(header.get_const(), Header::get_header_size());
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return Result::kFail;
    }
  }

  bytes = 0;
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.get_payload_size()) {
    const auto maybe_bytes =
        m_transport.write(&payload[bytes], header.get_payload_size() - bytes);
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return Result::kFail;
    }
  }

  return Result::kSuccess;
}

template <typename TTransport, typename THeaderData>
Result Connection<TTransport, THeaderData>::write(
    const u8* payload, typename Header::Payload_size payload_size,
    THeaderData header_data) {
  const auto payload_size = payload.size();
  if (payload_size > std::numeric_limits<Header::Payload_size>::max() ||
      payload_size < std::numeric_limits<Header::Payload_size>::min()) {
    // TODO send payloads larger than what can fit in a single packet
    DNET_ASSERT(
        payload_size > std::numeric_limits<Header::Payload_size>::max() ||
            payload_size < std::numeric_limits<Header::Payload_size>::min(),
        "Cannot fit the payload in the packet");
  }
  const Header header{static_cast<Header::Payload_size>(payload_size),
                      header_data};

  ssize_t bytes = 0;
  // TODO make it possible to break out of loops if bad header
  while (bytes < header.get_header_size()) {
    const auto maybe_bytes =
        m_transport.write(header.get_const(), Header::get_header_size());
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return Result::kFail;
    }
  }

  bytes = 0;
  while (bytes < header.get_payload_size()) {
    const auto maybe_value =
        m_transport.write(payload + bytes, payload_size - bytes);
    if (maybe_value.has_value()) {
      bytes += maybe_value.value();
    } else {
      return Result::kFail;
    }
  }

  return Result::kSuccess;
}

template <typename TTransport, typename THeaderData>
bool Connection<TTransport, THeaderData>::can_read() const {
  return m_transport.can_read();
}

template <typename TTransport, typename THeaderData>
bool Connection<TTransport, THeaderData>::can_write() const {
  return m_transport.can_write();
}

template <typename TTransport, typename THeaderData>
bool Connection<TTransport, THeaderData>::can_accept() const {
  return m_transport.can_accept();
}

template <typename TTransport, typename THeaderData>
bool Connection<TTransport, THeaderData>::has_error() const {
  return m_transport.has_error();
}

template <typename TTransport, typename THeaderData>
Result Connection<TTransport, THeaderData>::start_server(u16 port) {
  return m_transport.start_server(port);
}

template <typename TTransport, typename THeaderData>
std::optional<Connection<TTransport, THeaderData>>
Connection<TTransport, THeaderData>::accept() {
  auto maybe_transport = m_transport.accept();
  if (maybe_transport.has_value()) {
    return std::optional<Connection<TTransport, THeaderData>>{
        Connection(std::move(maybe_transport.value()))};
  }
  return std::nullopt;
}

}  // namespace dnet

#endif  // CONNECTION_HPP_
