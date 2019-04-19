#include "socket.hpp"

namespace dnet {

Socket::Socket(chif_net_protocol proto, chif_net_address_family fam)
    : socket_(CHIF_NET_INVALID_SOCKET),
      proto_(proto),
      af_(fam),
      last_error_(CHIF_NET_RESULT_SUCCESS) {}

Socket::Socket(Socket&& other) noexcept
    : socket_(other.socket_),
      proto_(other.proto_),
      af_(other.af_),
      last_error_(other.last_error_) {
  other.socket_ = CHIF_NET_INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) noexcept {
  if (this != &other) {
    socket_ = other.socket_;
    proto_ = other.proto_;
    af_ = other.af_;
    last_error_ = other.last_error_;
    other.socket_ = CHIF_NET_INVALID_SOCKET;
  }
  return *this;
}

Socket::Socket(chif_net_socket sock, chif_net_protocol proto,
               chif_net_address_family fam)
    : socket_(sock),
      proto_(proto),
      af_(fam),
      last_error_(CHIF_NET_RESULT_SUCCESS) {}

Result Socket::Open() {
  const auto res = chif_net_open_socket(&socket_, proto_, af_);
  if (res != CHIF_NET_RESULT_SUCCESS) {
    last_error_ = res;
  }
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

Result Socket::Bind(u16 port) {
  const auto res = chif_net_bind(socket_, port, af_);
  if (res != CHIF_NET_RESULT_SUCCESS) {
    last_error_ = res;
  }
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

Result Socket::Listen() {
  const auto res = chif_net_listen(socket_, CHIF_NET_DEFAULT_BACKLOG);
  if (res != CHIF_NET_RESULT_SUCCESS) {
    last_error_ = res;
  }
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

std::optional<Socket> Socket::Accept() {
  chif_net_address cli_address;
  chif_net_socket cli_sock;
  const auto res = chif_net_accept(socket_, &cli_address, &cli_sock);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<Socket>{Socket(cli_sock, proto_, af_)};
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<ssize_t> Socket::Read(u8* buf_out, size_t buflen) {
  ssize_t bytes;
  const auto res = chif_net_read(socket_, buf_out, buflen, &bytes);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<ssize_t>{bytes};
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<ssize_t> Socket::ReadFrom(u8* buf_out, size_t buflen,
                                        const std::string& addr, u16 port) {
  ssize_t bytes;
  chif_net_address target_addr;
  auto res =
      chif_net_create_address(&target_addr, addr.c_str(), port, af_);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    res = chif_net_readfrom(socket_, buf_out, buflen, &bytes, &target_addr);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      return std::optional<ssize_t>{bytes};
    }
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<ssize_t> Socket::Write(const u8* buf, size_t buflen) {
  ssize_t bytes;
  const auto res = chif_net_write(socket_, buf, buflen, &bytes);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<ssize_t>{bytes};
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<ssize_t> Socket::WriteTo(const u8* buf, size_t len,
                                       const std::string& addr, u16 port) {
  ssize_t bytes;
  chif_net_address target_addr;
  auto res =
      chif_net_create_address(&target_addr, addr.c_str(), port, af_);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    res = chif_net_writeto(socket_, buf, len, &bytes, &target_addr);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      return std::optional<ssize_t>{bytes};
    }
  }
  last_error_ = res;
  return std::nullopt;
}

void Socket::Close() { chif_net_close_socket(&socket_); }

Result Socket::Connect(const std::string& address, u16 port) {
  chif_net_address addr;
  const std::string portstr = std::to_string(port);
  auto res = chif_net_lookup_address(&addr, address.c_str(),
                                           portstr.c_str(), af_, proto_);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    if (socket_ != CHIF_NET_INVALID_SOCKET) {
      chif_net_close_socket(&socket_);
    }

    res = chif_net_open_socket(&socket_, proto_, af_);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      res = chif_net_connect(socket_, &addr);
      if (res == CHIF_NET_RESULT_SUCCESS) {
        return Result::kSuccess;
      }
    }
  }

  last_error_ = res;
  return Result::kFail;
}

bool Socket::CanWrite() const {
  int can;
  const auto res = chif_net_can_write(socket_, &can, 0);
  return res == CHIF_NET_RESULT_SUCCESS && can != 0;
}

bool Socket::CanRead() const {
  int can;
  const auto res = chif_net_can_read(socket_, &can, 0);
  return res == CHIF_NET_RESULT_SUCCESS && can != 0;
}

bool Socket::HasError() const {
  const auto res = chif_net_has_error(socket_);
  return (res != CHIF_NET_RESULT_SUCCESS);
}

std::optional<std::string> Socket::GetIp() {
  char ip[CHIF_NET_IPVX_STRING_LENGTH];
  const auto res =
      chif_net_ip_from_socket(socket_, ip, CHIF_NET_IPVX_STRING_LENGTH);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<std::string>{std::string(ip)};
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<u16> Socket::GetPort() {
  u16 port;
  const auto res = chif_net_port_from_socket(socket_, &port);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<u16>{port};
  }
  last_error_ = res;
  return std::nullopt;
}

std::tuple<Result, std::string, u16> Socket::GetPeer() {
  chif_net_address addr;
  auto res = chif_net_get_peer_address(socket_, &addr);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    char ip[CHIF_NET_IPVX_STRING_LENGTH];
    res = chif_net_ip_from_address(&addr, ip, CHIF_NET_IPVX_STRING_LENGTH);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      chif_net_port port;
      res = chif_net_port_from_address(&addr, &port);
      if (res == CHIF_NET_RESULT_SUCCESS) {
        return std::tuple<Result, std::string, u16>(Result::kSuccess,
                                                    std::string(ip), port);
      }
    }
  }
  last_error_ = res;
  return std::tuple<Result, std::string, u16>(Result::kFail, "", 0);
}

Result Socket::SetReuseAddr(bool reuse) const {
  const auto res = chif_net_set_reuse_addr(socket_, reuse);
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

Result Socket::SetBlocking(bool blocking) const {
  const auto res = chif_net_set_socket_blocking(socket_, blocking);
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

std::string Socket::LastErrorToString() const {
  const std::string error_string(chif_net_result_to_string(last_error_));
  return error_string;
}

}  // namespace dnet
