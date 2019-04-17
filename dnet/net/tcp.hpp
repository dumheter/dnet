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

#ifndef TCP_HPP_
#define TCP_HPP_

#include <optional>
#include <string>
#include <tuple>
#include "dnet/net/socket.hpp"
#include "dnet/util/result.hpp"
#include "dnet/util/types.hpp"

namespace dnet {

class Tcp {
 public:
  Tcp();

  // no copy
  Tcp(const Tcp& other) = delete;
  Tcp& operator=(const Tcp& other) = delete;

  Tcp(Tcp&& other) noexcept;
  Tcp& operator=(Tcp&& other) noexcept;

 private:
  explicit Tcp(Socket&& socket);

 public:
  Result start_server(u16 port);

  /*
   * Block until a client connects, call can_block() first to avoid blocking.
   */
  std::optional<Tcp> accept();

  Result connect(const std::string& address, u16 port) {
    return socket_.connect(address, port);
  }

  void disconnect() { socket_.close(); }

  std::optional<ssize_t> read(u8* buf_out, size_t buflen) {
    return socket_.read(buf_out, buflen);
  };

  std::optional<ssize_t> write(const u8* buf, size_t buflen) {
    return socket_.write(buf, buflen);
  };

  bool can_write() const { return socket_.can_write(); }

  bool can_read() const { return socket_.can_read(); }

  bool can_accept() const { return socket_.can_read(); }

  bool has_error() const { return socket_.has_error(); }

  std::optional<std::string> get_ip() { return socket_.get_ip(); }

  std::optional<u16> get_port() { return socket_.get_port(); }

  /**
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> get_peer() {
    return socket_.get_peer();
  }

  /**
   * If a Result comes back as kFail, or std::optional as std::nullopt, an
   * error will be set. Use this function to access the error string.
   */
  std::string last_error_to_string() const {
    return socket_.last_error_to_string();
  }

 private:
  Socket socket_;
};

}  // namespace dnet

#endif  // TCP_HPP_
