/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "socket.hpp"
#include <dnet/util/dnet_assert.hpp>

namespace dnet {

Socket::Socket(const TransportProtocol transport_protocol,
               const AddressFamily address_family)
    : socket_(CHIF_NET_INVALID_SOCKET),
      proto_(TransportProtocolToChifNet(transport_protocol)),
      af_(AddressFamilyToChifNet(address_family)),
      last_error_(CHIF_NET_RESULT_SUCCESS) {}

Socket::~Socket() {
  Close();
}

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

Socket::Socket(chif_net_socket& socket,
               const chif_net_transport_protocol transport_protocol,
               const chif_net_address_family address_family)
    : socket_(socket),
      proto_(transport_protocol),
      af_(address_family),
      last_error_(CHIF_NET_RESULT_SUCCESS) {
  socket = CHIF_NET_INVALID_SOCKET;
}

Result Socket::Open() {
  const auto res = chif_net_open_socket(&socket_, proto_, af_);
  if (res != CHIF_NET_RESULT_SUCCESS) {
    last_error_ = res;
  }
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

Result Socket::Bind(const u16 port) const {
  chif_net_address addr;
  const std::string portstr{std::to_string(port)};
  // TODO allow ipv6
  auto res =
      chif_net_create_address(&addr, "localhost", portstr.c_str(), af_, proto_);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    res = chif_net_bind(socket_, &addr);
  }
  if (res != CHIF_NET_RESULT_SUCCESS) {
    last_error_ = res;
  }
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

Result Socket::Listen() const {
  const auto res = chif_net_listen(socket_, CHIF_NET_DEFAULT_BACKLOG);
  if (res != CHIF_NET_RESULT_SUCCESS) {
    last_error_ = res;
  }
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

std::optional<Socket> Socket::Accept() const {
  chif_net_address cli_address;
  cli_address.address_family = af_;
  chif_net_socket cli_sock;
  const auto res = chif_net_accept(socket_, &cli_address, &cli_sock);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<Socket>{Socket(cli_sock, proto_, af_)};
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<int> Socket::Read(u8* buf_out, const size_t buflen) const {
  int bytes;
  const auto res = chif_net_read(socket_, buf_out, buflen, &bytes);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<int>{bytes};
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<int> Socket::ReadFrom(u8* buf_out, const size_t buflen,
                                        std::string& addr_out,
                                        u16& port_out) const {
  int bytes;
  chif_net_address source_addr;
  source_addr.address_family = af_;
  auto res = chif_net_readfrom(socket_, buf_out, buflen, &bytes, &source_addr);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    addr_out.resize(CHIF_NET_IPVX_STRING_LENGTH);
    res = chif_net_ip_from_address(&source_addr, addr_out.data(),
                                   addr_out.capacity());
    if (res == CHIF_NET_RESULT_SUCCESS) {
      res = chif_net_port_from_address(&source_addr, &port_out);
      if (res == CHIF_NET_RESULT_SUCCESS) {
        return std::optional<int>{bytes};
      }
    }
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<int> Socket::Write(const u8* buf, const size_t buflen) const {
  int bytes;
  const auto res = chif_net_write(socket_, buf, buflen, &bytes);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<int>{bytes};
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<int> Socket::WriteTo(const u8* buf, const size_t buflen,
                                       const std::string& addr,
                                       const u16 port) const {
  int bytes;
  chif_net_address target_addr;
  const std::string portstr = std::to_string(port);
  auto res = chif_net_create_address(&target_addr, addr.c_str(),
                                     portstr.c_str(), af_, proto_);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    res = chif_net_writeto(socket_, buf, buflen, &bytes, &target_addr);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      return std::optional<int>{bytes};
    }
  }
  last_error_ = res;
  return std::nullopt;
}

void Socket::Close() { chif_net_close_socket(&socket_); }

Result Socket::Connect(const std::string& address, const u16 port) {
  chif_net_address addr;
  const std::string portstr = std::to_string(port);
  auto res = chif_net_create_address(&addr, address.c_str(), portstr.c_str(),
                                     af_, proto_);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    if (socket_ != CHIF_NET_INVALID_SOCKET) {
      Close();
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
  const auto res = chif_net_has_error(socket_, 0);
  return (res != CHIF_NET_RESULT_SUCCESS);
}

std::optional<std::string> Socket::GetIp() const {
  char ip[CHIF_NET_IPVX_STRING_LENGTH];
  const auto res =
      chif_net_ip_from_socket(socket_, ip, CHIF_NET_IPVX_STRING_LENGTH);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<std::string>{std::string(ip)};
  }
  last_error_ = res;
  return std::nullopt;
}

std::optional<u16> Socket::GetPort() const {
  u16 port;
  const auto res = chif_net_port_from_socket(socket_, &port);
  if (res == CHIF_NET_RESULT_SUCCESS) {
    return std::optional<u16>{port};
  }
  last_error_ = res;
  return std::nullopt;
}

std::tuple<Result, std::string, u16> Socket::GetPeer() const {
  chif_net_address addr;
  addr.address_family = af_;
  auto res = chif_net_peer_address_from_socket(socket_, &addr);
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

Result Socket::SetReuseAddr(const bool reuse) const {
  const auto res = chif_net_set_reuse_addr(socket_, reuse);
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

Result Socket::SetBlocking(const bool blocking) const {
  const auto res = chif_net_set_blocking(socket_, blocking);
  return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
}

std::string Socket::LastErrorToString() const {
  const std::string error_string(chif_net_result_to_string(last_error_));
  return error_string;
}

}  // namespace dnet
