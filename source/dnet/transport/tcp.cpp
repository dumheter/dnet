// ============================================================ //
// Headers
// ============================================================ //

#include "tcp.hpp"

// ============================================================ //
// Class Definition
// ============================================================ //

namespace dnet
{

  Tcp::Tcp() : m_socket(CHIF_NET_PROTOCOL_TCP, CHIF_NET_ADDRESS_FAMILY_IPV4)
  {
  }

  Tcp::Tcp(Tcp&& other) noexcept
    : m_socket(std::move(other.m_socket))
  {
  }

  Tcp& Tcp::operator=(Tcp&& other) noexcept
  {
    if (this != &other) {
      m_socket = std::move(other.m_socket);
    }
    return *this;
  }

  Tcp::Tcp(Socket&& socket)
    : m_socket(std::move(socket))
  {
  }

  void Tcp::start_server(u16 port)
  {
    m_socket.set_reuse_addr(true);
    m_socket.bind(port);
    m_socket.listen();
  }

  Tcp Tcp::accept()
  {
    auto sock = m_socket.accept();
    return Tcp(std::move(sock));
  }

  void Tcp::connect(const std::string& ip, u16 port)
  {
    m_socket.connect(ip, port);
  }

  void Tcp::disconnect()
  {
    m_socket.close();
  }

}
