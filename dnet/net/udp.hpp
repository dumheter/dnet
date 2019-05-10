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

#ifndef UDP_HPP_
#define UDP_HPP_

#include <dnet/net/socket.hpp>
#include <dnet/util/result.hpp>
#include <dnet/util/types.hpp>
#include <optional>
#include <string>
#include <tuple>

namespace dnet {

class Udp {
 public:
  Udp();

  // no copy
  Udp(const Udp& other) = delete;
  Udp& operator=(const Udp& other) = delete;

  Udp(Udp&& other) noexcept;
  Udp& operator=(Udp&& other) noexcept;

 private:
  explicit Udp(Socket&& socket);

 public:
  /**
   * Start listening on @port. Will also open the socket.
   */
  Result StartServer(const u16 port);

  Result Open() { return socket_.Open(); }

  Result Connect(const std::string& address, const u16 port) {
    return socket_.Connect(address, port);
  }

  void Disconnect() { socket_.Close(); }

  /**
   * @return Amount of read bytes, or nullopt on failure.
   */
  std::optional<ssize_t> Read(u8* buf_out, const size_t buflen) {
    return socket_.Read(buf_out, buflen);
  };

  std::optional<ssize_t> ReadFrom(u8* buf_out, const size_t buflen,
                                  std::string& addr_out, u16& port_out) {
    return socket_.ReadFrom(buf_out, buflen, addr_out, port_out);
  }

  /**
   * @return Amount of written bytes, or nullopt on failure.
   */
  std::optional<ssize_t> Write(const u8* buf, const size_t buflen) {
    return socket_.Write(buf, buflen);
  };

  std::optional<ssize_t> WriteTo(const u8* buf, const size_t buflen,
                                 const std::string& addr, const u16 port) {
    return socket_.WriteTo(buf, buflen, addr, port);
  }

  bool CanWrite() const { return socket_.CanWrite(); }

  bool CanRead() const { return socket_.CanRead(); }

  bool HasError() const { return socket_.HasError(); }

  std::optional<std::string> GetIp() { return socket_.GetIp(); }

  std::optional<u16> GetPort() { return socket_.GetPort(); }

  /**
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> GetPeer() { return socket_.GetPeer(); }

  /**
   * If a Result comes back as kFail, or std::optional as std::nullopt, an
   * error will be set. Use this function to access the error string.
   */
  std::string LastErrorToString() const { return socket_.LastErrorToString(); }

  Result SetBlocking(const bool blocking) const {
    return socket_.SetBlocking(blocking);
  }

 private:
  Socket socket_;
};

}  // namespace dnet

#endif  // UDP_HPP_
