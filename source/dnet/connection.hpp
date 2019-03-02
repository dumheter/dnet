#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include <limits>
#include <string>
#include <optional>
#include "dnet/header/packet_header.hpp"
#include "dnet/payload/payload.hpp"
#include "dnet/transport/tcp.hpp"
#include "dnet/util/types.hpp"
#include "dnet/util/result.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

/**
 * TODO Make payload container be part of the template
 */
namespace dnet {
template <typename TTransport = Tcp, typename THeader = Packet_header,
          u64 MAX_PAYLOAD_SIZE =
              std::numeric_limits<decltype(THeader::Header_meta::size)>::max()>
class Connection {
 public:
  // ====================================================================== //
  // Lifetime
  // ====================================================================== //

  Connection();
  explicit Connection(TTransport&& transport);
  Connection(TTransport&& transport, THeader&& packet);

  Connection(const Connection& other) = delete;
  Connection& operator=(const Connection& other) = delete;

  Connection(Connection&& other) noexcept;
  Connection& operator=(Connection&& other) noexcept;

  // ====================================================================== //
  // Client methods
  // ====================================================================== //

  Result connect(const std::string& ip, u16 port);

  void disconnect();

  std::optional<payload_container> read();
  Result read(payload_container& payload_out);
  // void read(u8* payload, size_t payload_size);

  Result write(const payload_container& payload);
  Result write(const u8* payload, size_t payload_size);

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

  std::string last_error_to_string() const { return m_transport.last_error_to_string(); }

  // ====================================================================== //
  // Private members
  // ====================================================================== //

 private:
  TTransport m_transport;
  THeader m_header;

  static constexpr u64 MIN_PAYLOAD_SIZE = 4048;
};

// ====================================================================== //
// template definition
// ====================================================================== //

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::Connection()
    : m_transport(), m_header() {}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::Connection(
    TTransport&& transport)
    : m_transport(std::move(transport)), m_header() {}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::Connection(
    TTransport&& transport, TPacket&& packet)
    : m_transport(transport), m_header(packet) {}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::Connection(
    Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>&& other) noexcept
    : m_transport(std::move(other.m_transport)),
      m_header(std::move(other.m_header)) {}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>&
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::operator=(
    Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>&& other) noexcept {
  if (&other != this) {
    m_transport = std::move(other.m_transport);
    m_header = std::move(other.m_header);
  }

  return *this;
}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
Result Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::connect(
    const std::string& ip, u16 port) {
  return m_transport.connect(ip, port);
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
void Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::disconnect() {
  m_transport.disconnect();
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
std::optional<payload_container>
Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::read() {
  payload_container payload{};
  const Result res = read(payload);
  if (res == Result::kSuccess) {
    return std::optional<payload_container>{payload};
  }
  return std::nullopt;
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
Result
Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::read(payload_container& payload_out) {
  ssize_t bytes = 0;
  // TODO make it possible to break out of loops if bad header
  // TODO bad cast
  while (static_cast<size_t>(bytes) < m_header.get_header_size()) {
    const auto maybe_bytes =
        m_transport.read(m_header.get(), m_header.get_header_size());
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    }
    else {
      return Result::kFail;
    }
  }

  if (payload_out.capacity() < m_header.get_payload_size()) {
    payload_out.reserve(m_header.get_payload_size());
  }

  payload_out.resize(payload_out.capacity());
  bytes = 0;
  // TODO bad cast
  while (static_cast<size_t>(bytes) < m_header.get_payload_size()) {
    // TODO timeout read in case bad info in header
    const auto maybe_bytes =
        m_transport.read(&payload_out[bytes], m_header.get_payload_size() - bytes);
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    }
    else {
      return Result::kFail;
    }
  }
  payload_out.resize(static_cast<size_t>(bytes));
  return Result::kSuccess;
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
Result Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::write(
    const payload_container& payload) {
  const Result res = m_header.build_header(payload.size());
  if (res == Result::kSuccess) {

    ssize_t bytes = 0;
    // TODO make it possible to break out of loops if bad header
    // TODO bad cast
    while (static_cast<size_t>(bytes) < m_header.get_header_size()) {
      const auto maybe_bytes =
          m_transport.write(m_header.get(), m_header.get_header_size());
      if (maybe_bytes.has_value()) {
        bytes += maybe_bytes.value();
      }
      else {
        return Result::kFail;
      }
    }

    bytes = 0;
    // TODO bad cast
    while (static_cast<size_t>(bytes) < m_header.get_payload_size()) {
      const auto maybe_bytes =
          m_transport.write(payload.data(), payload.size());
      if (maybe_bytes.has_value()) {
        bytes += maybe_bytes.value();
      }
      else {
        return Result::kFail;
      }
    }
  }
  return Result::kSuccess;
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
Result Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::write(
    const u8* payload, size_t payload_size) {
  const Result res = m_header.build_header(payload_size);
  if (res == Result::kSuccess) {

    ssize_t bytes = 0;
    // TODO make it possible to break out of loops if bad header
    while (bytes < m_header.get_header_size()) {
      const auto maybe_bytes =
          m_transport.write(m_header.get(), m_header.get_header_size());
      if (maybe_bytes.has_value()) {
        bytes += maybe_bytes.value();
      }
      else {
        return Result::kFail;
      }
    }

    bytes = 0;
    while (bytes < m_header.get_payload_size()) {
      const auto maybe_value =
          m_transport.write(payload, payload_size);
      if (maybe_value.has_value()) {
        bytes += maybe_value.value();
      }
      else {
        return Result::kFail;
      }
    }
  }
  return Result::kSuccess;
}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
bool Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::can_read() const {
  return m_transport.can_read();
}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
bool Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::can_write() const {
  return m_transport.can_write();
}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
bool Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::can_accept() const {
  return m_transport.can_accept();
}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
bool Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::has_error() const {
  return m_transport.has_error();
}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
Result Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::start_server(u16 port) {
  return m_transport.start_server(port);
}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
std::optional<Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>>
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::accept() {
  auto maybe_transport = m_transport.accept();
  if (maybe_transport.has_value()) {
    return std::optional<Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>>{
      Connection(std::move(maybe_transport.value()))
    };
  }
  return std::nullopt;
}

}  // namespace dnet

#endif  // CONNECTION_HPP_
