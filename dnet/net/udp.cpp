#include "udp.hpp"

namespace dnet {

Udp::Udp() : socket_(CHIF_NET_PROTOCOL_UDP, CHIF_NET_ADDRESS_FAMILY_IPV4) {}

Udp::Udp(Udp&& other) noexcept : socket_(std::move(other.socket_)) {}

Udp& Udp::operator=(Udp&& other) noexcept {
  if (this != &other) {
    socket_ = std::move(other.socket_);
  }
  return *this;
}

Udp::Udp(Socket&& socket) : socket_(std::move(socket)) {}

Result Udp::start_server(u16 port) {
  Result res = socket_.open();
  if (res == Result::kSuccess) {
    res = socket_.set_reuse_addr(true);
    if (res == Result::kSuccess) {
      res = socket_.bind(port);
      if (res == Result::kSuccess) {
        return Result::kSuccess;
      }
    }
  }
  return Result::kFail;
}

}
