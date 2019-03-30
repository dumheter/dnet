// ============================================================ //
// Headers
// ============================================================ //

#include "tcp.hpp"

// ============================================================ //
// Class Definition
// ============================================================ //

namespace dnet {

Tcp::Tcp() : m_socket(CHIF_NET_PROTOCOL_TCP, CHIF_NET_ADDRESS_FAMILY_IPV4) {}

Tcp::Tcp(Tcp&& other) noexcept : m_socket(std::move(other.m_socket)) {}

Tcp& Tcp::operator=(Tcp&& other) noexcept {
  if (this != &other) {
    m_socket = std::move(other.m_socket);
  }
  return *this;
}

Tcp::Tcp(Socket&& socket) : m_socket(std::move(socket)) {}

Result Tcp::start_server(u16 port) {
  Result res = m_socket.open();
  if (res == Result::kSuccess) {
    // After closing the program, the port can be left in an occupied state,
    // setting resue to true allows for instant reuse of that port.
    res = m_socket.set_reuse_addr(true);
    if (res == Result::kSuccess) {
      res = m_socket.bind(port);
      if (res == Result::kSuccess) {
        res = m_socket.listen();
        if (res == Result::kSuccess) {
          return Result::kSuccess;
        }
      }
    }
  }
  return Result::kFail;
}

std::optional<Tcp> Tcp::accept() {
  auto maybe_socket = m_socket.accept();
  if (maybe_socket.has_value()) {
    return std::optional<Tcp>{Tcp(std::move(maybe_socket.value()))};
  }
  return std::nullopt;
}

}  // namespace dnet
