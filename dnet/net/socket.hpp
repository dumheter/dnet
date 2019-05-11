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

#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include <dnet/util/result.hpp>
#include <dnet/util/types.hpp>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

namespace dnet {

enum class TransportProtocol { kUdp, kTcp };

enum class AddressFamily { kIPv4, kIPv6 };

class Socket {
 public:
  Socket(const TransportProtocol transport, const AddressFamily address_family);

  ~Socket();

  Socket(const Socket& other) = delete;
  Socket& operator=(const Socket& other) = delete;

  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other) noexcept;

  Result Open() const;

  Result Bind(const u16 port) const;

  Result Listen() const;

  std::optional<Socket> Accept() const;

  /**
   * @return Amount of read bytes, or nullopt on failure.
   */
  std::optional<ssize_t> Read(u8* buf_out, const size_t buflen) const;

  /**
   * Places the address and port in the addr_out and port_out fields.
   */
  std::optional<ssize_t> ReadFrom(u8* buf_out, const size_t buflen,
                                  std::string& addr_out, u16& port_out) const;
  /**
   * @return Amount of written bytes, or nullopt on failure.
   */
  std::optional<ssize_t> Write(const u8* buf, const size_t buflen) const;

  std::optional<ssize_t> WriteTo(const u8* buf, const size_t buflen,
                                 const std::string& addr, const u16 port) const;

  void Close() const;

  Result Connect(const std::string& address, u16 port) const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool CanWrite() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool CanRead() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool HasError() const;

  /**
   * @return Ip address, or nullopt on failure.
   */
  std::optional<std::string> GetIp() const;

  /**
   * @return Port, or nullopt on failure.
   */
  std::optional<u16> GetPort() const;

  /**
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> GetPeer() const;

  Result SetReuseAddr(const bool reuse) const;

  Result SetBlocking(const bool blocking) const;

  std::string LastErrorToString() const;

 private:
  class Pimpl;
  std::unique_ptr<Pimpl> pimpl_;

  Socket(Pimpl&& pimpl);
};

}  // namespace dnet

#endif  // SOCKET_HPP_
