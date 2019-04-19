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

#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include <optional>
#include <string>
#include <tuple>
#include <chif_net/chif_net.h>
#include <dnet/util/result.hpp>
#include <dnet/util/types.hpp>

namespace dnet {

class Socket {
 public:
  Socket(chif_net_protocol proto, chif_net_address_family fam);

  ~Socket() { chif_net_close_socket(&socket_); };

  Socket(const Socket& other) = delete;
  Socket& operator=(const Socket& other) = delete;

  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other) noexcept;

 private:
  /*
   * Create a Socket from an existing open chif_net_socket
   */
  Socket(chif_net_socket sock, chif_net_protocol proto,
         chif_net_address_family fam);

 public:
  Result Open();

  Result Bind(u16 port);

  Result Listen();

  std::optional<Socket> Accept();

  /**
   * @return Amount of read bytes, or nullopt on failure.
   */
  std::optional<ssize_t> Read(u8* buf_out, size_t buflen);

  std::optional<ssize_t> ReadFrom(u8* buf_out, size_t buflen,
                                  const std::string& addr, u16 port);
  /**
   * @return Amount of written bytes, or nullopt on failure.
   */
  std::optional<ssize_t> Write(const u8* buf, size_t buflen);

  std::optional<ssize_t> WriteTo(const u8* buf, size_t len,
                                 const std::string& addr, u16 port);
  void Close();

  Result Connect(const std::string& address, u16 port);

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
  std::optional<std::string> GetIp();

  /**
   * @return Port, or nullopt on failure.
   */
  std::optional<u16> GetPort();

  /**
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> GetPeer();

  Result SetReuseAddr(bool reuse) const;

  Result SetBlocking(bool blocking) const;

  std::string LastErrorToString() const;

 private:
  chif_net_socket socket_;
  chif_net_protocol proto_;
  chif_net_address_family af_;
  chif_net_result last_error_;
};

}  // namespace dnet

#endif  // SOCKET_HPP_
