// ============================================================ //
// Headers
// ============================================================ //

#include "socket.hpp"
#include <iostream>

// ============================================================ //
// Class Definition
// ============================================================ //

namespace dnet
{

  Socket::Socket(chif_net_protocol proto, chif_net_address_family fam)
    : m_socket(CHIF_NET_INVALID_SOCKET), m_proto(proto), m_fam(fam)
  {
    auto res = chif_net_open_socket(&m_socket, proto, fam);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("unable to open socket");
    }
  }

  Socket::Socket(Socket&& other) noexcept
  {
    m_socket = other.m_socket;
    m_proto = other.m_proto;
    m_fam = other.m_fam;
    other.m_socket = CHIF_NET_INVALID_SOCKET;
  }

  Socket& Socket::operator=(Socket&& other) noexcept
  {
    if (this != &other) {
      m_socket = other.m_socket;
      m_proto = other.m_proto;
      m_fam = other.m_fam;
      other.m_socket = CHIF_NET_INVALID_SOCKET;
    }
    return *this;
  }

  Socket::Socket(chif_net_socket sock, chif_net_protocol proto, chif_net_address_family fam)
    : m_socket(sock), m_proto(proto), m_fam(fam)
  {
  }

  void Socket::bind(u16 port) const
  {
    auto res = chif_net_bind(m_socket, port, m_fam);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to bind socket");
    }
  }

  void Socket::listen() const
  {
      auto res = chif_net_listen(m_socket, CHIF_NET_DEFAULT_BACKLOG);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to listen");
    }
  }

  Socket Socket::accept() const
  {
    chif_net_address cli_address;
    chif_net_socket cli_sock;
    auto res = chif_net_accept(m_socket, &cli_address, &cli_sock);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to accept socket: " + std::string(chif_net_result_to_string(res)));
    }

    return Socket(cli_sock, m_proto, m_fam);
  }

  ssize_t Socket::read(u8* buf, size_t len) const
  {
    ssize_t bytes;
    auto res = chif_net_read(m_socket, buf, len, &bytes);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to read from socket");
    }

    return bytes;
  }

  ssize_t Socket::write(const u8* buf, size_t len) const
  {
    ssize_t bytes;
    auto res = chif_net_write(m_socket, buf, len, &bytes);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to write to socket");
    }

    return bytes;
  }

  void Socket::close()
  {
    chif_net_close_socket(&m_socket);
  }

  void Socket::connect(const std::string& ip, u16 port)
  {
    chif_net_address addr;
    auto res = chif_net_create_address(&addr, ip.c_str(), port, m_fam);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to create chif_net address");
    }

    if (m_socket == CHIF_NET_INVALID_SOCKET) {
      res = chif_net_open_socket(&m_socket, m_proto, m_fam);
      if (res != CHIF_NET_RESULT_SUCCESS) {
        throw socket_exception("failed to open socket when attempting to connect");
      }
    }
    res = chif_net_connect(m_socket, &addr);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("falied to connect to remote");
    }
  }

  bool Socket::can_write() const
  {
    int can;
    const auto res = chif_net_can_write(m_socket, &can, 0);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to check if we can write");
    }

    return can != 0;
  }

  bool Socket::can_read() const
  {
    int can;
    const auto res = chif_net_can_read(m_socket, &can, 0);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to check if we can read");
    }

    return can != 0;
  }

  bool Socket::has_error() const
  {
    const auto res = chif_net_has_error(m_socket);
    return (res != CHIF_NET_RESULT_SUCCESS);
  }

    std::string Socket::get_ip() const
  {
    char ip[CHIF_NET_IPVX_STRING_LENGTH];
    auto res = chif_net_ip_from_socket(m_socket, ip, CHIF_NET_IPVX_STRING_LENGTH);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to get ip");
    }

    return std::string(ip);
  }

  u16 Socket::get_port() const
  {
    u16 port;
    auto res = chif_net_port_from_socket(m_socket, &port);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to get port");
    }

    return port;
  }

  std::string Socket::get_remote_ip() const
  {
    char ip[CHIF_NET_IPVX_STRING_LENGTH];
    auto res = chif_net_ip_from_socket(m_socket, ip, CHIF_NET_IPVX_STRING_LENGTH);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to get remote ip");
    }

    return std::string(ip);
  }

  u16 Socket::get_remote_port() const
  {
    u16 port;
    auto res = chif_net_port_from_socket(m_socket, &port);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to get remote port");
    }

    return port;
  }

  void Socket::set_reuse_addr(bool reuse) const
  {
    auto res = chif_net_set_reuse_addr(m_socket, reuse);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to set reuse addr");
    }
  }

  void Socket::set_blocking(bool blocking) const
  {
    auto res = chif_net_set_socket_blocking(m_socket, blocking);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to set blocking");
    }
  }

}
