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

#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <dnet/net/packet_header.hpp>
#include <dnet/util/dnet_assert.hpp>
#include <dnet/util/result.hpp>
#include <dnet/util/types.hpp>

namespace dnet {

/**
 * TODO Make payload container be part of the template?
 * @tparam TVector A container that has the functionality of std::vector<u8>.
 * @tparam TTransport
 * @tparam THeaderData Provide your own data in the header! Note, packet size is
 * handled internally.
 */
template <typename TVector, typename TTransport, typename THeaderData = HeaderDataExample>
class Connection {
 public:
  static_assert(std::is_standard_layout<THeaderData>::value,
                "THeaderData must be trivial (C struct) in order to serialize "
                "correctly.");

  using Header = PacketHeader<THeaderData>;

  // ====================================================================== //
  // Lifetime
  // ====================================================================== //

  Connection();
  explicit Connection(TTransport&& transport);

  // no copy
  Connection(const Connection& other) = delete;
  Connection& operator=(const Connection& other) = delete;

  Connection(Connection&& other) noexcept;
  Connection& operator=(Connection&& other) noexcept;

  ~Connection() = default;

  // ====================================================================== //
  // Client methods
  // ====================================================================== //

  Result Connect(const std::string& address, u16 port);

  void Disconnect();

  std::tuple<Result, THeaderData> Read(TVector& payload_out);

  Result Write(const THeaderData& header_data, const TVector& payload);

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

  Result StartServer(u16 port);

  std::optional<Connection> Accept();

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

  std::optional<std::string> GetIp() { return transport_.GetIp(); }

  std::optional<u16> GetPort() { return transport_.GetPort(); }

  /**
   * Get the address information about the peer which we are connected to.
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> GetPeer() {
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
  TTransport transport_;
};

// ====================================================================== //
// template definition
// ====================================================================== //

template <typename TVector, typename TTransport, typename THeaderData>
Connection<TVector, TTransport, THeaderData>::Connection() : transport_() {}

// TODO use std::forward here?
template <typename TVector, typename TTransport, typename THeaderData>
Connection<TVector, TTransport, THeaderData>::Connection(TTransport&& transport)
    : transport_(std::move(transport)) {}

template <typename TVector, typename TTransport, typename THeaderData>
Connection<TVector, TTransport, THeaderData>::Connection(
    Connection<TVector, TTransport, THeaderData>&& other) noexcept
    : transport_(std::move(other.transport_)) {}

template <typename TVector, typename TTransport, typename THeaderData>
Connection<TVector, TTransport, THeaderData>& Connection<TVector, TTransport, THeaderData>::
operator=(Connection<TVector, TTransport, THeaderData>&& other) noexcept {
  if (&other != this) {
    transport_ = std::move(other.transport_);
  }
  return *this;
}

template <typename TVector, typename TTransport, typename THeaderData>
Result Connection<TVector, TTransport, THeaderData>::Connect(
    const std::string& address, u16 port) {
  return transport_.Connect(address, port);
}

template <typename TVector, typename TTransport, typename THeaderData>
void Connection<TVector, TTransport, THeaderData>::Disconnect() {
  transport_.Disconnect();
}

// TODO go over the types used
// TODO utilize NRVO
template <typename TVector, typename TTransport, typename THeaderData>
std::tuple<Result, THeaderData> Connection<TVector, TTransport, THeaderData>::Read(TVector& payload_out) {
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
    const auto maybe_bytes = transport_.Read(
        &payload_out[bytes], header.payload_size() - bytes);
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
template <typename TVector, typename TTransport, typename THeaderData>
Result Connection<TVector, TTransport, THeaderData>::Write(
    const THeaderData& header_data, const TVector& payload) {
  const auto payload_size = payload.size();
  if (payload_size > std::numeric_limits<typename Header::PayloadSize>::max() ||
      payload_size < std::numeric_limits<typename Header::PayloadSize>::min()) {
    // TODO send payloads larger than what can fit in a single packet
    DNET_ASSERT(
        payload_size > std::numeric_limits<typename Header::PayloadSize>::max() ||
            payload_size < std::numeric_limits<typename Header::PayloadSize>::min(),
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

template <typename TVector, typename TTransport, typename THeaderData>
bool Connection<TVector, TTransport, THeaderData>::CanRead() const {
  return transport_.CanRead();
}

template <typename TVector, typename TTransport, typename THeaderData>
bool Connection<TVector, TTransport, THeaderData>::CanWrite() const {
  return transport_.CanWrite();
}

template <typename TVector, typename TTransport, typename THeaderData>
bool Connection<TVector, TTransport, THeaderData>::CanAccept() const {
  return transport_.CanAccept();
}

template <typename TVector, typename TTransport, typename THeaderData>
bool Connection<TVector, TTransport, THeaderData>::HasError() const {
  return transport_.HasError();
}

template <typename TVector, typename TTransport, typename THeaderData>
Result Connection<TVector, TTransport, THeaderData>::StartServer(u16 port) {
  return transport_.StartServer(port);
}

template <typename TVector, typename TTransport, typename THeaderData>
std::optional<Connection<TVector, TTransport, THeaderData>>
Connection<TVector, TTransport, THeaderData>::Accept() {
  auto maybe_transport = transport_.Accept();
  if (maybe_transport.has_value()) {
    return std::optional<Connection<TVector, TTransport, THeaderData>>{
        Connection(std::move(maybe_transport.value()))};
  }
  return std::nullopt;
}

}  // namespace dnet

#endif  // CONNECTION_HPP_
