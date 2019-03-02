// ============================================================ //
// Headers
// ============================================================ //

#include "socket.hpp"

// ============================================================ //
// Class Definition
// ============================================================ //

namespace dnet {

Socket::Socket(chif_net_protocol proto, chif_net_address_family fam)
    : m_socket(CHIF_NET_INVALID_SOCKET), m_proto(proto), m_fam(fam),
      m_last_error(CHIF_NET_RESULT_SUCCESS) {}

Socket::Socket(Socket&& other) noexcept
    : m_socket(other.m_socket), m_proto(other.m_proto), m_fam(other.m_fam),
      m_last_error(other.m_last_error){
  other.m_socket = CHIF_NET_INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) noexcept {
  if (this != &other) {
    m_socket = other.m_socket;
    m_proto = other.m_proto;
    m_fam = other.m_fam;
    m_last_error = other.m_last_error;
    other.m_socket = CHIF_NET_INVALID_SOCKET;
  }
  return *this;
}

Socket::Socket(chif_net_socket sock, chif_net_protocol proto,
               chif_net_address_family fam)
    : m_socket(sock), m_proto(proto), m_fam(fam),
      m_last_error(CHIF_NET_RESULT_SUCCESS){}

Result Socket::open() {
  const auto res = chif_net_open_socket(&m_socket, m_proto, m_fam);
  if (res != CHIF_NET_RESULT_SUCCESS) { m_last_error = res; }
  return (res == CHIF_NET_RESULT_SUCCESS ?
          Result::kSuccess : Result::kFail);
}

Result Socket::bind(u16 port) {
  const auto res = chif_net_bind(m_socket, port, m_fam);
  if (res != CHIF_NET_RESULT_SUCCESS) { m_last_error = res; }
  return (res == CHIF_NET_RESULT_SUCCESS ?
          Result::kSuccess : Result::kFail);
}

Result Socket::listen() {
  const auto res = chif_net_listen(m_socket, CHIF_NET_DEFAULT_BACKLOG);
  if (res != CHIF_NET_RESULT_SUCCESS) { m_last_error = res; }
  return (res == CHIF_NET_RESULT_SUCCESS ?
          Result::kSuccess : Result::kFail);
}

std::optional<Socket> Socket::accept() {
  chif_net_address cli_address;
  chif_net_socket cli_sock;
  const auto res = chif_net_accept(m_socket, &cli_address, &cli_sock);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<Socket>{Socket(cli_sock, m_proto, m_fam)};
  }
  m_last_error = res;
  return std::nullopt;
}

std::optional<ssize_t> Socket::read(u8* buf_out, size_t buflen) {
  ssize_t bytes;
  const auto res = chif_net_read(m_socket, buf_out, buflen, &bytes);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<ssize_t>{bytes};
  }
  m_last_error = res;
  return std::nullopt;
}

std::optional<ssize_t> Socket::write(const u8* buf, size_t len) {
  ssize_t bytes;
  const auto res = chif_net_write(m_socket, buf, len, &bytes);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<ssize_t>{bytes};
  }
  m_last_error = res;
  return std::nullopt;
}

void Socket::close() { chif_net_close_socket(&m_socket); }

Result Socket::connect(const std::string& ip, u16 port) {
  chif_net_address addr;
  auto res = chif_net_create_address(&addr, ip.c_str(), port, m_fam);
  if (res == CHIF_NET_RESULT_SUCCESS) {

    if (m_socket != CHIF_NET_INVALID_SOCKET) {
      chif_net_close_socket(&m_socket);
    }

    res = chif_net_open_socket(&m_socket, m_proto, m_fam);
    if (res == CHIF_NET_RESULT_SUCCESS) {

      res = chif_net_connect(m_socket, &addr);
      if (res == CHIF_NET_RESULT_SUCCESS) {
        return Result::kSuccess;
      }
    }
  }

  m_last_error = res;
  return Result::kFail;
}

bool Socket::can_write() const {
  int can;
  const auto res = chif_net_can_write(m_socket, &can, 0);
  return res == CHIF_NET_RESULT_SUCCESS && can != 0;
}

bool Socket::can_read() const {
  int can;
  const auto res = chif_net_can_read(m_socket, &can, 0);
  return res == CHIF_NET_RESULT_SUCCESS && can != 0;
}

bool Socket::has_error() const {
  const auto res = chif_net_has_error(m_socket);
  return (res != CHIF_NET_RESULT_SUCCESS);
}

std::optional<std::string> Socket::get_ip() {
  char ip[CHIF_NET_IPVX_STRING_LENGTH];
  const auto res = chif_net_ip_from_socket(m_socket, ip, CHIF_NET_IPVX_STRING_LENGTH);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<std::string>{std::string(ip)};
  }
  m_last_error = res;
  return std::nullopt;
}

std::optional<u16> Socket::get_port() {
  u16 port;
  const auto res = chif_net_port_from_socket(m_socket, &port);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<u16>{port};
  }
  m_last_error = res;
  return std::nullopt;
}

Result Socket::set_reuse_addr(bool reuse) const {
  const auto res = chif_net_set_reuse_addr(m_socket, reuse);
  return (res == CHIF_NET_RESULT_SUCCESS ?
          Result::kSuccess : Result::kFail);
}

Result Socket::set_blocking(bool blocking) const {
  const auto res = chif_net_set_socket_blocking(m_socket, blocking);
  return (res == CHIF_NET_RESULT_SUCCESS ?
          Result::kSuccess : Result::kFail);
}

std::string Socket::last_error_to_string() const {
  const std::string error_string(chif_net_result_to_string(m_last_error));
  return error_string;
}

}  // namespace dnet
