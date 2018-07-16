#ifndef NET_CONNECTION_HPP
#define NET_CONNECTION_HPP

// ============================================================ //
// Headers
// ============================================================ //

#include <net/util/types.hpp>
#include <net/payload/payload.hpp>
#include <net/util/dnet_exception.hpp>
#include <string>

// ====================================================================== //
// Exception Class
// ====================================================================== //

namespace dnet
{

  class connection_exception : public dnet_exception
  {

  public:

    explicit connection_exception(const std::string& what)
      : dnet_exception(what) {};

    explicit connection_exception(const char* what)
      : dnet_exception(what) {};

  };

}

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  template <typename TTransport, typename THeader>
  class Connection
  {

  public:

    // ====================================================================== //
    // Lifetime
    // ====================================================================== //

    Connection();
    explicit Connection(TTransport&& transport);
    Connection(TTransport&& transport, THeader&& packet);

    Connection(const Connection<TTransport, THeader>& other) = delete;
    Connection& operator=(const Connection<TTransport, THeader>& other) = delete;

    Connection(Connection<TTransport, THeader>&& other) noexcept;
    Connection& operator=(Connection<TTransport, THeader>&& other) noexcept;

    // ====================================================================== //
    // Client methods
    // ====================================================================== //

    void connect(const std::string& ip, u16 port);

    void read(payload_container& payload);

    void write(payload_container& payload);

    inline std::string get_remote_ip() { return m_transport.get_remote_ip(); }

    inline u16 get_remote_port() { return m_transport.get_remote_port(); }

    // ====================================================================== //
    // Server methods
    // ====================================================================== //

    void start_server(u16 port);

    Connection accept();

    // ====================================================================== //
    // util methods
    // ====================================================================== //



    // ====================================================================== //
    // Private members
    // ====================================================================== //

  private:

    TTransport m_transport;
    THeader m_header;

    static constexpr u64 MIN_PAYLOAD_SIZE = 4048;
    static constexpr u64 MAX_PAYLOAD_SIZE = std::numeric_limits<decltype(m_header.size)>::max();

  };

  // ====================================================================== //
  // template definition
  // ====================================================================== //

  template<typename TTransport, typename TPacket>
  Connection<TTransport, TPacket>::Connection()
  : m_transport(), m_header()
  {

  }

  template<typename TTransport, typename TPacket>
  Connection<TTransport, TPacket>::Connection(TTransport&& transport)
    : m_transport(std::move(transport)), m_header()
  {

  }

  template<typename TTransport, typename TPacket>
  Connection<TTransport, TPacket>::Connection(TTransport&& transport, TPacket&& packet)
  : m_transport(transport), m_header(packet)
  {

  }

  template<typename TTransport, typename TPacket>
  Connection<TTransport, TPacket>::Connection(Connection<TTransport, TPacket>&& other) noexcept
  {
    m_transport = std::move(other.m_transport);
    m_header = std::move(other.m_header);
  }

  template<typename TTransport, typename TPacket>
  Connection<TTransport, TPacket>& Connection<TTransport, TPacket>::operator=(Connection<TTransport, TPacket>&& other) noexcept
  {
    if (&other != this) {
      m_transport = std::move(other.m_transport);
      m_header = std::move(other.m_header);
    }

    return *this;
  }

  template<typename TTransport, typename TPacket>
  void Connection<TTransport, TPacket>::connect(const std::string& ip, u16 port)
  {
    m_transport.connect(ip, port);
  }

  template<typename TTransport, typename THeader>
  void Connection<TTransport, THeader>::read(payload_container& payload)
  {
    // todo make it possible to break out of loops if bad header

    ssize_t bytes = 0;
    while (bytes < m_header.get_header_size()) {
      bytes += m_transport.read(m_header.get(), m_header.get_header_size());
    }

    if (m_header.is_valid()) {
      if (payload.capacity() < MIN_PAYLOAD_SIZE)
        payload.reserve(MIN_PAYLOAD_SIZE);
      if (payload.capacity() < m_header.get_payload_size())
        payload.reserve(m_header.get_payload_size());

      payload.resize(payload.capacity());
      bytes = 0;
      while (bytes < m_header.get_payload_size()) {
        // todo timeout read in case bad into in header
        bytes += m_transport.read(&payload[bytes], payload.capacity() - bytes);
      }
      payload.resize(bytes);
    }
    else {
      throw connection_exception("invalid header found");
    }
  }

  template<typename TTransport, typename THeader>
  void Connection<TTransport, THeader>::write(payload_container& payload)
  {
    // todo make it possible to break out of loops if bad header

    m_header.build_header(payload);

    ssize_t bytes = 0;
    while (bytes < m_header.get_header_size()) {
      bytes += m_transport.write(m_header.get(), m_header.get_header_size());
    }

    bytes = 0;
    while (bytes < m_header.get_payload_size()) {
      bytes += m_transport.write(&payload[0], payload.size());
    }
  }

  template<typename TTransport, typename TPacket>
  void Connection<TTransport, TPacket>::start_server(u16 port)
  {
    m_transport.start_server(port);
  }

  template<typename TTransport, typename TPacket>
  Connection<TTransport, TPacket> Connection<TTransport, TPacket>::accept()
  {
    Connection<TTransport, TPacket> client{m_transport.accept()};
    return client;
  }

}

#endif //NET_CONNECTION_HPP
