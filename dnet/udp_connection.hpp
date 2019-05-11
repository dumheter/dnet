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

#ifndef UDP_CONNECTION_HPP_
#define UDP_CONNECTION_HPP_

#include <dnet/net/udp.hpp>
#include <dnet/util/dnet_assert.hpp>
#include <dnet/util/result.hpp>
#include <dnet/util/types.hpp>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace dnet {

/**
 * Header, to be placed in front of each payload.
 */
struct UdpHeader {
  u32 id;
  u32 acked_id;
  u32 acked_bitmask;
};

/**
 * A buffer that keeps sizeof(UdpHeader) bytes of room in the front.
 * @tparam TVector A std::vector<u8> like container type.
 */
template <typename TVector>
class UdpBuffer {
 public:
  UdpBuffer() = default;
  ~UdpBuffer() = default;

 private:
  TVector vector_{};
};

/**
 * @tparam TVector Same TVector as in UdpBuffer, see it for more info.
 */
template <typename TVector>
class UdpConnection {
  // ============================================================ //
  // Lifetime
  // ============================================================ //

 public:
  UdpConnection();
  explicit UdpConnection(Udp&& transport);

  // no copy
  UdpConnection(const UdpConnection& other) = delete;
  UdpConnection& operator=(const UdpConnection& other) = delete;

  UdpConnection(UdpConnection&& other) noexcept;
  UdpConnection& operator=(UdpConnection&& other) noexcept;

  ~UdpConnection() = default;

  // ============================================================ //
  // Client
  // ============================================================ //

  Result Connect(const std::string& address, u16 port) const;

  void Disconnect() const;

  /**
   * Read incoming packet and put in @buffer_out.
   */
  Result Read(UdpBuffer<TVector>& buffer_out) const;

  /**
   * Prepare the header, then copy over the buffer to kernel memory to
   * be sent of to remote.
   */
  Result Write(UdpBuffer<TVector>& buffer) const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool CanRead() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool CanWrite() const;

  // ============================================================ //
  // Server
  // ============================================================ //

  /**
   * Start listening for incoming packets on port @port.
   */
  Result StartServer(u16 port) const;

  // ============================================================ //
  // Misc
  // ============================================================ //

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool HasError() const;

  std::optional<std::string> GetIp() const { return transport_.GetIp(); }

  std::optional<u16> GetPort() const { return transport_.GetPort(); }

  /**
   * Get the address information about the peer which we are connected to.
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> GetPeer() const {
    return transport_.GetPeer();
  }

  std::string LastErrorToSting() const {
    return transport_.LastErrorToString();
  }

  Result SetBlocking(bool blocking) const {
    return transport_.SetBlocking(blocking);
  }

  // ============================================================ //
  // Data
  // ============================================================ //

 private:
  Udp transport_;
};

// ============================================================ //
// template definition
// ============================================================ //

template <typename TVector>
UdpConnection<TVector>::UdpConnection() : transport_() {}

template <typename TVector>
UdpConnection<TVector>::UdpConnection(Udp&& transport)
    : transport_(std::forward<Udp>(transport)) {}

template <typename TVector>
UdpConnection<TVector>::UdpConnection(UdpConnection&& other) noexcept
    : transport_(std::move(other)) {}

template <typename TVector>
UdpConnection<TVector>& UdpConnection<TVector>::operator=(
    UdpConnection&& other) noexcept {
  if (this != &other) {
    transport_ = std::move(other.transport_);
  }
  return *this;
}

template <typename TVector>
Result UdpConnection<TVector>::Connect(const std::string& address,
                                       u16 port) const {
  return transport_.Connect(address, port);
}

template <typename TVector>
void UdpConnection<TVector>::Disconnect() const {
  transport_.Disconnect();
}

template <typename TVector>
Result UdpConnection<TVector>::Read(UdpBuffer<TVector>& buffer_out) const {
  // TODO
  buffer_out.resize(buffer_out.capacity());
  const auto res = transport_.Read(reinterpret_cast<u8*>(buffer_out.data()),
                                   buffer_out.size());
  if (res.has_value()) {
    buffer_out.resize(res.value());
  }
  return res;
}

template <typename TVector>
Result Write(UdpBuffer<TVector>& buffer);

template <typename TVector>
Result UdpConnection<TVector>::StartServer(u16 port) const {
  return transport_.StartServer(port);
}
}  // namespace dnet

#endif  // UDP_CONNECTION_HPP_
