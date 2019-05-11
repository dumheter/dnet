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

#ifndef TCP_CONNECTION_HPP_
#define TCP_CONNECTION_HPP_

#include <dnet/net/packet_header.hpp>
#include <dnet/net/tcp.hpp>
#include <dnet/util/dnet_assert.hpp>
#include <dnet/util/result.hpp>
#include <dnet/util/types.hpp>
#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

namespace dnet {

/**
 * TODO Make payload container be part of the template?
 * @tparam TVector A container that has the functionality of std::vector<u8>.
 * @tparam THeaderData Provide your own data in the header! Note, packet size is
 * handled internally.
 */
template <typename TVector, typename THeaderData = HeaderDataExample>
class TcpConnection {
 public:
  static_assert(std::is_standard_layout<THeaderData>::value,
                "THeaderData must be trivial (C struct) in order to serialize "
                "correctly.");

  using Header = PacketHeader<THeaderData>;

  // ====================================================================== //
  // Lifetime
  // ====================================================================== //

  TcpConnection();
  explicit TcpConnection(Tcp&& transport);

  // no copy
  TcpConnection(const TcpConnection& other) = delete;
  TcpConnection& operator=(const TcpConnection& other) = delete;

  TcpConnection(TcpConnection&& other) noexcept;
  TcpConnection& operator=(TcpConnection&& other) noexcept;

  ~TcpConnection() = default;

  // ====================================================================== //
  // Client methods
  // ====================================================================== //

  Result Connect(const std::string& address, u16 port) const;

  void Disconnect() const;

  std::tuple<Result, THeaderData> Read(TVector& payload_out) const;

  Result Write(const THeaderData& header_data, const TVector& payload) const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool CanRead() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool CanWrite() const;

  // ====================================================================== //
  // Server methods
  // ====================================================================== //

  Result StartServer(u16 port) const;

  std::optional<TcpConnection> Accept() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool CanAccept() const;

  // ====================================================================== //
  // General methods
  // ====================================================================== //

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

  std::string LastErrorToString() const {
    return transport_.LastErrorToString();
  }

  Result SetBlocking(bool blocking) const {
    return transport_.SetBlocking(blocking);
  }

  // ====================================================================== //
  // Data members
  // ====================================================================== //

 private:
  Tcp transport_;
};

// ====================================================================== //
// template definition
// ====================================================================== //

template <typename TVector, typename THeaderData>
TcpConnection<TVector, THeaderData>::TcpConnection() : transport_() {}

// TODO use std::forward here?
template <typename TVector, typename THeaderData>
TcpConnection<TVector, THeaderData>::TcpConnection(Tcp&& transport)
    : transport_(std::move(transport)) {}

template <typename TVector, typename THeaderData>
TcpConnection<TVector, THeaderData>::TcpConnection(
    TcpConnection<TVector, THeaderData>&& other) noexcept
    : transport_(std::move(other.transport_)) {}

template <typename TVector, typename THeaderData>
TcpConnection<TVector, THeaderData>& TcpConnection<TVector, THeaderData>::
operator=(TcpConnection<TVector, THeaderData>&& other) noexcept {
  if (&other != this) {
    transport_ = std::move(other.transport_);
  }
  return *this;
}

template <typename TVector, typename THeaderData>
Result TcpConnection<TVector, THeaderData>::Connect(const std::string& address,
                                                    u16 port) const {
  return transport_.Connect(address, port);
}

template <typename TVector, typename THeaderData>
void TcpConnection<TVector, THeaderData>::Disconnect() const {
  transport_.Disconnect();
}

// TODO go over the types used
// TODO utilize NRVO
template <typename TVector, typename THeaderData>
std::tuple<Result, THeaderData> TcpConnection<TVector, THeaderData>::Read(
    TVector& payload_out) const {
  Header header{};
  ssize_t bytes = 0;
  // TODO make it possible to break out of loops if bad header
  while (bytes < Header::header_size()) {
    const auto maybe_bytes =
        transport_.Read(header.get(), Header::header_size());
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return std::make_tuple<Result, THeaderData>(Result::kFail, THeaderData{});
    }
  }

  if (payload_out.capacity() < header.payload_size()) {
    payload_out.reserve(header.payload_size());
  }

  payload_out.resize(payload_out.capacity());
  bytes = 0;
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.payload_size()) {
    // TODO timeout read in case bad info in header
    const auto maybe_bytes =
        transport_.Read(&payload_out[bytes], header.payload_size() - bytes);
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return std::make_tuple<Result, THeaderData>(Result::kFail,
                                                  header.header_data());
    }
  }
  payload_out.resize(static_cast<size_t>(bytes));
  return std::make_tuple<Result, THeaderData>(Result::kSuccess,
                                              header.header_data());
}

// TODO utilize NRVO
template <typename TVector, typename THeaderData>
Result TcpConnection<TVector, THeaderData>::Write(
    const THeaderData& header_data, const TVector& payload) const {
  const auto payload_size = payload.size();
  if (payload_size > std::numeric_limits<typename Header::PayloadSize>::max() ||
      payload_size < std::numeric_limits<typename Header::PayloadSize>::min()) {
    // TODO send payloads larger than what can fit in a single packet
    dnet_assert(
        payload_size >
                std::numeric_limits<typename Header::PayloadSize>::max() ||
            payload_size <
                std::numeric_limits<typename Header::PayloadSize>::min(),
        "Cannot fit the payload in the packet");
  }
  const Header header{static_cast<typename Header::PayloadSize>(payload_size),
                      header_data};

  ssize_t bytes = 0;
  // TODO make it possible to break out of loops if bad header
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.header_size()) {
    const auto maybe_bytes =
        transport_.Write(header.get(), Header::header_size());
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return Result::kFail;
    }
    // TODO not using bytes
  }

  bytes = 0;
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.payload_size()) {
    const auto maybe_bytes =
        transport_.Write(&payload[bytes], header.payload_size() - bytes);
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return Result::kFail;
    }
  }

  return Result::kSuccess;
}

template <typename TVector, typename THeaderData>
bool TcpConnection<TVector, THeaderData>::CanRead() const {
  return transport_.CanRead();
}

template <typename TVector, typename THeaderData>
bool TcpConnection<TVector, THeaderData>::CanWrite() const {
  return transport_.CanWrite();
}

template <typename TVector, typename THeaderData>
bool TcpConnection<TVector, THeaderData>::CanAccept() const {
  return transport_.CanAccept();
}

template <typename TVector, typename THeaderData>
bool TcpConnection<TVector, THeaderData>::HasError() const {
  return transport_.HasError();
}

template <typename TVector, typename THeaderData>
Result TcpConnection<TVector, THeaderData>::StartServer(u16 port) const {
  return transport_.StartServer(port);
}

template <typename TVector, typename THeaderData>
std::optional<TcpConnection<TVector, THeaderData>>
TcpConnection<TVector, THeaderData>::Accept() const {
  auto maybe_transport = transport_.Accept();
  if (maybe_transport.has_value()) {
    return std::optional<TcpConnection<TVector, THeaderData>>{
        TcpConnection(std::move(maybe_transport.value()))};
  }
  return std::nullopt;
}

}  // namespace dnet

#endif  // TCP_CONNECTION_HPP_
