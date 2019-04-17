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

Result Tcp::start_server(u16 port) {
  Result res = socket_.open();
  if (res == Result::kSuccess) {
    // After closing the program, the port can be left in an occupied state,
    // setting resue to true allows for instant reuse of that port.
    res = socket_.set_reuse_addr(true);
    if (res == Result::kSuccess) {
      res = socket_.bind(port);
      if (res == Result::kSuccess) {
        res = socket_.listen();
        if (res == Result::kSuccess) {
          return Result::kSuccess;
        }
      }
    }
  }
  return Result::kFail;
}

std::optional<Tcp> Tcp::accept() {
  auto maybe_socket = socket_.accept();
  if (maybe_socket.has_value()) {
    return std::optional<Tcp>{Tcp(std::move(maybe_socket.value()))};
  }
  return std::nullopt;
}

}  // namespace dnet
