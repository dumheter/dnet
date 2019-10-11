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

#include "tcp.hpp"

namespace dnet {

Tcp::Tcp() : socket_(TransportProtocol::kTcp, AddressFamily::kIPv4) {}

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

std::optional<Tcp> Tcp::Accept() const {
  auto maybe_socket = socket_.Accept();
  if (maybe_socket.has_value()) {
    return std::optional<Tcp>{Tcp(std::move(maybe_socket.value()))};
  }
  return std::nullopt;
}

}  // namespace dnet
