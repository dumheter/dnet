#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include <limits>
#include <string>
#include "dnet/header/packet_header.hpp"
#include "dnet/payload/payload.hpp"
#include "dnet/transport/tcp.hpp"
#include "dnet/util/dnet_exception.hpp"
#include "dnet/util/types.hpp"

// ====================================================================== //
// Exception Class
// ====================================================================== //

namespace dnet {

class connection_exception : public dnet_exception {
 public:
  explicit connection_exception(const std::string& what);

  explicit connection_exception(const char* what);
};

}  // namespace dnet

// ============================================================ //
// Class Declaration
// ============================================================ //

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
  // Connection(Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>&& other)
  // noexcept;
  Connection& operator=(Connection&& other) noexcept;

  // ====================================================================== //
  // Client methods
  // ====================================================================== //

  void connect(const std::string& ip, u16 port);

  void disconnect();

  void read(payload_container& payload_out);
  // void read(u8* payload, size_t payload_size);

  void write(const payload_container& payload);
  void write(const u8* payload, size_t payload_size);

  bool can_read() const;

  bool can_write() const;

  bool can_accept() const;

  bool has_error() const;

  // ====================================================================== //
  // Server methods
  // ====================================================================== //

  void start_server(u16 port);

  Connection accept();

  // ====================================================================== //
  // General methods
  // ====================================================================== //

  inline std::string get_ip() const { return m_transport.get_ip(); }

  inline u16 get_port() const { return m_transport.get_port(); }

  inline std::string get_remote_ip() const {
    return m_transport.get_remote_ip();
  }

  inline u16 get_remote_port() const { return m_transport.get_remote_port(); }

  // ====================================================================== //
  // Private members
  // ====================================================================== //

 private:
  TTransport m_transport;
  THeader m_header;

  static constexpr u64 MIN_PAYLOAD_SIZE = 4048;
  // static constexpr u64 MAX_PAYLOAD_SIZE =
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
void Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::connect(
    const std::string& ip, u16 port) {
  m_transport.connect(ip, port);
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
void Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::disconnect() {
  m_transport.disconnect();
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
void Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::read(
    payload_container& payload) {
  // todo make it possible to break out of loops if bad header

  ssize_t bytes = 0;
  while (static_cast<size_t>(bytes) < m_header.get_header_size()) {
    bytes += m_transport.read(m_header.get(), m_header.get_header_size());
  }

  if (payload.capacity() < MIN_PAYLOAD_SIZE) payload.reserve(MIN_PAYLOAD_SIZE);
  if (payload.capacity() < m_header.get_payload_size())
    payload.reserve(m_header.get_payload_size());

  payload.resize(payload.capacity());
  bytes = 0;
  while (static_cast<size_t>(bytes) < m_header.get_payload_size()) {
    // todo timeout read in case bad into in header
    bytes +=
        m_transport.read(&payload[bytes], m_header.get_payload_size() - bytes);
  }
  payload.resize(static_cast<size_t>(bytes));
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
void Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::write(
    const payload_container& payload) {
  // todo make it possible to break out of loops if bad header

  m_header.build_header(payload.size());

  ssize_t bytes = 0;
  while (static_cast<size_t>(bytes) < m_header.get_header_size()) {
    bytes += m_transport.write(m_header.get(), m_header.get_header_size());
  }

  bytes = 0;
  while (static_cast<size_t>(bytes) < m_header.get_payload_size()) {
    bytes += m_transport.write(&payload[0], payload.size());
  }
}

template <typename TTransport, typename THeader, u64 MAX_PAYLOAD_SIZE>
void Connection<TTransport, THeader, MAX_PAYLOAD_SIZE>::write(
    const u8* payload, size_t payload_size) {
  // todo make it possible to break out of loops if bad header

  m_header.build_header(payload_size);

  ssize_t bytes = 0;
  while (bytes < m_header.get_header_size()) {
    bytes += m_transport.write(m_header.get(), m_header.get_header_size());
  }

  bytes = 0;
  while (bytes < m_header.get_payload_size()) {
    bytes += m_transport.write(payload, payload_size);
  }
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
void Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::start_server(u16 port) {
  m_transport.start_server(port);
}

template <typename TTransport, typename TPacket, u64 MAX_PAYLOAD_SIZE>
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>
Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE>::accept() {
  Connection<TTransport, TPacket, MAX_PAYLOAD_SIZE> client{
      m_transport.accept()};
  return client;
}

}  // namespace dnet

#endif  // CONNECTION_HPP_
