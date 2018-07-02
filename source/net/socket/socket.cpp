// ============================================================ //
// Headers
// ============================================================ //

#include "socket.hpp"

// ============================================================ //
// Class Definition
// ============================================================ //

namespace dnet
{

  Socket::Socket(chif_net_protocol proto, chif_net_address_family fam)
    : m_proto(proto), m_fam(fam)
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

  void Socket::bind(u16 port)
  {
    auto res = chif_net_bind(m_socket, port, m_fam);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to bind socket");
    }
  }

  void Socket::listen()
  {
    auto res = chif_net_listen(m_socket, CHIF_NET_DEFAULT_MAXIMUM_BACKLOG);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to listen");
    }
  }

  Socket Socket::accept()
  {
    chif_net_address cli_address;
    auto cli_sock = chif_net_accept(m_socket, &cli_address);

    if (cli_sock == CHIF_NET_INVALID_SOCKET) {
      throw socket_exception("failed to accept socket");
    }

    return Socket(cli_sock, m_proto, m_fam);
  }

  ssize_t Socket::read(u8* buf, size_t len)
  {
    ssize_t bytes;
    auto res = chif_net_read(m_socket, buf, len, &bytes);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to read from socket");
    }

    return bytes;
  }

  ssize_t Socket::write(u8* buf, size_t len)
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

    res = chif_net_connect(m_socket, addr);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("falied to connect to remote");
    }
  }

  bool Socket::can_write()
  {
    bool can;
    auto res = chif_net_can_write(m_socket, &can);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to check if we can write");
    }

    return can;
  }

  bool Socket::can_read()
  {
    bool can;
    auto res = chif_net_can_read(m_socket, &can);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to check if we can read");
    }

    return can;
  }

  std::string Socket::get_remote_ip()
  {
    char ip[CHIF_NET_IPVX_STRING_LENGTH];
    auto res = chif_net_get_peer_name(m_socket, ip, CHIF_NET_IPVX_STRING_LENGTH);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to get remote ip");
    }

    return std::string(ip);
  }

  u16 Socket::get_remote_port()
  {
    u16 port;
    auto res = chif_net_get_peer_port(m_socket, &port);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to get remote port");
    }

    return port;
  }

  void Socket::set_reuse_addr(bool reuse)
  {
    auto res = chif_net_set_reuse_addr(m_socket, reuse);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to set reuse addr");
    }
  }

  void Socket::set_blocking(bool blocking)
  {
    auto res = chif_net_set_socket_blocking(m_socket, blocking);

    if (res != CHIF_NET_RESULT_SUCCESS) {
      throw socket_exception("failed to set blocking");
    }
  }
}
