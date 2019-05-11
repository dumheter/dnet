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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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
#include <chif_net/chif_net.h>
#include <dnet/util/dnet_assert.hpp>

namespace dnet {

// ============================================================ //
// chif_net Pointer to implementation
// ============================================================ //

/**
 * Pimpl pattern used mainly to avoid bringing the windows.h header
 * into all other headers
 */
class Socket::Pimpl {
 public:
  Pimpl(const chif_net_protocol proto, const chif_net_address_family af)
      : socket_(CHIF_NET_INVALID_SOCKET),
        proto_(proto),
        af_(af),
        last_error_(CHIF_NET_RESULT_SUCCESS) {}

  ~Pimpl() { chif_net_close_socket(&socket_); }

  Pimpl(Pimpl&& other) noexcept
      : socket_(other.socket_),
        proto_(other.proto_),
        af_(other.af_),
        last_error_(other.last_error_) {
    other.socket_ = CHIF_NET_INVALID_SOCKET;
  }

  Pimpl& operator=(Pimpl&& other) noexcept {
    if (this != &other) {
      socket_ = other.socket_;
      proto_ = other.proto_;
      af_ = other.af_;
      last_error_ = other.last_error_;
      other.socket_ = CHIF_NET_INVALID_SOCKET;
    }
    return *this;
  }

  Pimpl(chif_net_socket& sock, const chif_net_protocol proto,
        const chif_net_address_family af)
      : socket_(sock),
        proto_(proto),
        af_(af),
        last_error_(CHIF_NET_RESULT_SUCCESS) {
    sock = CHIF_NET_INVALID_SOCKET;
  }

  Result Open() {
    const auto res = chif_net_open_socket(&socket_, proto_, af_);
    if (res != CHIF_NET_RESULT_SUCCESS) {
      last_error_ = res;
    }
    return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
  }

  Result Bind(const u16 port) {
    const auto res = chif_net_bind(socket_, port, af_);
    if (res != CHIF_NET_RESULT_SUCCESS) {
      last_error_ = res;
    }
    return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
  }

  Result Listen() {
    const auto res = chif_net_listen(socket_, CHIF_NET_DEFAULT_BACKLOG);
    if (res != CHIF_NET_RESULT_SUCCESS) {
      last_error_ = res;
    }
    return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
  }

  std::optional<Pimpl> Accept() {
    chif_net_address cli_address;
    chif_net_socket cli_sock;
    const auto res = chif_net_accept(socket_, &cli_address, &cli_sock);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      return std::optional<Pimpl>{Pimpl(cli_sock, proto_, af_)};
    }
    last_error_ = res;
    return std::nullopt;
  }

  std::optional<ssize_t> Read(u8* buf_out, const size_t buflen) {
    ssize_t bytes;
    const auto res = chif_net_read(socket_, buf_out, buflen, &bytes);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      return std::optional<ssize_t>{bytes};
    }
    last_error_ = res;
    return std::nullopt;
  }

  std::optional<ssize_t> ReadFrom(u8* buf_out, const size_t buflen,
                                  std::string& addr_out, u16& port_out) {
    ssize_t bytes;
    chif_net_address source_addr;
    auto res = chif_net_readfrom(socket_, buf_out, buflen, &bytes, &source_addr);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      addr_out.resize(CHIF_NET_IPVX_STRING_LENGTH);
      res = chif_net_ip_from_address(&source_addr, addr_out.data(),
                                     addr_out.capacity());
      if (res == CHIF_NET_RESULT_SUCCESS) {
        res = chif_net_port_from_address(&source_addr, &port_out);
        if (res == CHIF_NET_RESULT_SUCCESS) {
          return std::optional<ssize_t>{bytes};
        }
      }
    }
    last_error_ = res;
    return std::nullopt;
  }

  std::optional<ssize_t> Write(const u8* buf, const size_t buflen) {
    ssize_t bytes;
    const auto res = chif_net_write(socket_, buf, buflen, &bytes);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      return std::optional<ssize_t>{bytes};
    }
    last_error_ = res;
    return std::nullopt;
  }

  std::optional<ssize_t> WriteTo(const u8* buf, const size_t buflen,
                                 const std::string& addr, const u16 port) {
    ssize_t bytes;
    chif_net_address target_addr;
    const std::string portstr = std::to_string(port);
    auto res = chif_net_lookup_address(&target_addr, addr.c_str(),
                                       portstr.c_str(), af_, proto_);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      res = chif_net_writeto(socket_, buf, buflen, &bytes, &target_addr);
      if (res == CHIF_NET_RESULT_SUCCESS) {
        return std::optional<ssize_t>{bytes};
      }
    }
    last_error_ = res;
    return std::nullopt;
  }

  void Close() { chif_net_close_socket(&socket_); }

  Result Connect(const std::string& address, const u16 port) {
    chif_net_address addr;
    const std::string portstr = std::to_string(port);
    auto res = chif_net_lookup_address(&addr, address.c_str(), portstr.c_str(),
                                       af_, proto_);
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

  bool CanWrite() const {
    int can;
    const auto res = chif_net_can_write(socket_, &can, 0);
    return res == CHIF_NET_RESULT_SUCCESS && can != 0;
  }

  bool CanRead() const {
    int can;
    const auto res = chif_net_can_read(socket_, &can, 0);
    return res == CHIF_NET_RESULT_SUCCESS && can != 0;
  }

  bool HasError() const {
    const auto res = chif_net_has_error(socket_);
    return (res != CHIF_NET_RESULT_SUCCESS);
  }

  std::optional<std::string> GetIp() {
    char ip[CHIF_NET_IPVX_STRING_LENGTH];
    const auto res =
        chif_net_ip_from_socket(socket_, ip, CHIF_NET_IPVX_STRING_LENGTH);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      return std::optional<std::string>{std::string(ip)};
    }
    last_error_ = res;
    return std::nullopt;
  }

  std::optional<u16> GetPort() {
    u16 port;
    const auto res = chif_net_port_from_socket(socket_, &port);
    if (res == CHIF_NET_RESULT_SUCCESS) {
      return std::optional<u16>{port};
    }
    last_error_ = res;
    return std::nullopt;
  }

  std::tuple<Result, std::string, u16> GetPeer() {
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

  Result SetReuseAddr(const bool reuse) const {
    const auto res = chif_net_set_reuse_addr(socket_, reuse);
    return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
  }

  Result SetBlocking(const bool blocking) const {
    const auto res = chif_net_set_socket_blocking(socket_, blocking);
    return (res == CHIF_NET_RESULT_SUCCESS ? Result::kSuccess : Result::kFail);
  }

  std::string LastErrorToString() const {
    const std::string error_string(chif_net_result_to_string(last_error_));
    return error_string;
  }

 private:
  chif_net_socket socket_;
  chif_net_protocol proto_;
  chif_net_address_family af_;
  chif_net_result last_error_;
};

// ============================================================ //
// Helper Functions
// ============================================================ //

static chif_net_protocol TranslateTransportProtocol(const TransportProtocol transport_protocol) {
  switch (transport_protocol) {
    case TransportProtocol::kTcp:
      return CHIF_NET_PROTOCOL_TCP;
    case TransportProtocol::kUdp:
      return CHIF_NET_PROTOCOL_UDP;
  }
  dnet_assert(false, "switch statement did not cover all TransportProtocol options");
  return CHIF_NET_PROTOCOL_TCP;
}

static chif_net_address_family TranslateAddressFamily(const AddressFamily address_family) {
  switch (address_family) {
    case AddressFamily::kIPv4:
      return CHIF_NET_ADDRESS_FAMILY_IPV4;
    case AddressFamily::kIPv6:
      return CHIF_NET_ADDRESS_FAMILY_IPV6;
  }
  dnet_assert(false,
              "switch statement did not cover all AddressFamily options");
  return CHIF_NET_ADDRESS_FAMILY_IPV4;
}

// ============================================================ //
// Socket Definition
// ============================================================ //

Socket::Socket(const TransportProtocol transport,
               const AddressFamily address_family)
    : pimpl_(std::make_unique<Pimpl>(TranslateTransportProtocol(transport),
                                     TranslateAddressFamily(address_family))) {}

Socket::~Socket() = default;

Socket::Socket(Socket&& other) noexcept
    : pimpl_(std::make_unique<Pimpl>(std::move(*other.pimpl_))) {}

Socket& Socket::operator=(Socket&& other) noexcept {
  if (this != &other) {
    pimpl_ = std::move(other.pimpl_);
  }
  return *this;
}

// TODO removed if you can, keep in pimpl only
// Socket::Socket(chif_net_socket sock, chif_net_protocol proto,
//                chif_net_address_family fam)
//     : socket_(sock),
//       proto_(proto),
//       af_(fam),
//       last_error_(CHIF_NET_RESULT_SUCCESS) {}

Result Socket::Open() {
  return pimpl_->Open();
}

Result Socket::Bind(const u16 port) {
  return pimpl_->Bind(port);
}

Result Socket::Listen() {
  return pimpl_->Listen();
}

std::optional<Socket> Socket::Accept() {
  auto maybe_pimpl = pimpl_->Accept();
  if (maybe_pimpl.has_value()) {
    return std::optional<Socket>{Socket{std::move(maybe_pimpl.value())}};
  }
  else {
    return std::nullopt;
  }
}

std::optional<ssize_t> Socket::Read(u8* buf_out, const size_t buflen) {
  return pimpl_->Read(buf_out, buflen);
}

std::optional<ssize_t> Socket::ReadFrom(u8* buf_out, const size_t buflen,
                                        std::string& addr_out, u16& port_out) {
  return pimpl_->ReadFrom(buf_out, buflen, addr_out, port_out);
}

std::optional<ssize_t> Socket::Write(const u8* buf, const size_t buflen) {
  return pimpl_->Write(buf, buflen);
}

std::optional<ssize_t> Socket::WriteTo(const u8* buf, const size_t buflen,
                                       const std::string& addr,
                                       const u16 port) {
  return pimpl_->WriteTo(buf, buflen, addr, port);
}

void Socket::Close() { pimpl_->Close(); }

Result Socket::Connect(const std::string& address, const u16 port) {
  return pimpl_->Connect(address, port);
}

bool Socket::CanWrite() const {
  return pimpl_->CanWrite();
}

bool Socket::CanRead() const {
  return pimpl_->CanRead();
}

bool Socket::HasError() const {
  return pimpl_->HasError();
}

std::optional<std::string> Socket::GetIp() {
  return pimpl_->GetIp();
}

std::optional<u16> Socket::GetPort() {
  return pimpl_->GetPort();
}

std::tuple<Result, std::string, u16> Socket::GetPeer() {
  return pimpl_->GetPeer();
}

Result Socket::SetReuseAddr(const bool reuse) const {
  return pimpl_->SetReuseAddr(reuse);
}

Result Socket::SetBlocking(const bool blocking) const {
  return pimpl_->SetBlocking(blocking);
}

std::string Socket::LastErrorToString() const {
  return pimpl_->LastErrorToString();
}

Socket::Socket(Pimpl&& pimpl_)
    : pimpl_(std::make_unique<Pimpl>(std::move(pimpl_))) {}

}  // namespace dnet
