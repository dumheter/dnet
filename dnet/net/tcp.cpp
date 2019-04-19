#include "tcp.hpp"

namespace dnet {

Tcp::Tcp() : socket_(CHIF_NET_PROTOCOL_TCP, CHIF_NET_ADDRESS_FAMILY_IPV4) {}

Tcp::Tcp(Tcp&& other) noexcept : socket_(std::move(other.socket_)) {}

Tcp& Tcp::operator=(Tcp&& other) noexcept {
  if (this != &other) {
    socket_ = std::move(other.socket_);
  }
  return *this;
}

Tcp::Tcp(Socket&& socket) : socket_(std::move(socket)) {}

Result Tcp::StartServer(u16 port) {
  Result res = socket_.Open();
  if (res == Result::kSuccess) {
    // After closing the program, the port can be left in an occupied state,
    // setting resue to true allows for instant reuse of that port.
    res = socket_.SetReuseAddr(true);
    if (res == Result::kSuccess) {
      res = socket_.Bind(port);
      if (res == Result::kSuccess) {
        res = socket_.Listen();
        if (res == Result::kSuccess) {
          return Result::kSuccess;
        }
      }
    }
  }
  return Result::kFail;
}

std::optional<Tcp> Tcp::Accept() {
  auto maybe_socket = socket_.Accept();
  if (maybe_socket.has_value()) {
    return std::optional<Tcp>{Tcp(std::move(maybe_socket.value()))};
  }
  return std::nullopt;
}

}  // namespace dnet
