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
template <typename TVector, typename TTransport, typename THeaderData = Header_data_example>
class Connection {
 public:
  static_assert(std::is_standard_layout<THeaderData>::value,
                "THeaderData must be trivial (C struct) in order to serialize "
                "correctly.");

  using Header = Packet_header<THeaderData>;

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

  Result connect(const std::string& address, u16 port);

  void disconnect();

  std::tuple<Result, THeaderData> read(TVector& payload_out);

  Result write(const THeaderData& header_data, const TVector& payload);

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool can_read() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool can_write() const;

  // ====================================================================== //
  // Server methods
  // ====================================================================== //

  Result start_server(u16 port);

  std::optional<Connection> accept();

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool can_accept() const;

  // ====================================================================== //
  // General methods
  // ====================================================================== //

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool has_error() const;

  std::optional<std::string> get_ip() { return m_transport.get_ip(); }

  std::optional<u16> get_port() { return m_transport.get_port(); }

  std::string last_error_to_string() const {
    return m_transport.last_error_to_string();
  }

  /**
   * Get the address information about the peer which we are connected to.
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> get_peer() {
    return m_transport.get_peer();
  }

  Result set_blocking(bool blocking) const {
    return m_transport.set_blocking(blocking);
  }

  // ====================================================================== //
  // Data members
  // ====================================================================== //

 private:
  TTransport m_transport;
};

// ====================================================================== //
// template definition
// ====================================================================== //

template <typename TVector, typename TTransport, typename THeaderData>
Connection<TVector, TTransport, THeaderData>::Connection() : m_transport() {}

template <typename TVector, typename TTransport, typename THeaderData>
Connection<TVector, TTransport, THeaderData>::Connection(TTransport&& transport)
    : m_transport(std::move(transport)) {}

template <typename TVector, typename TTransport, typename THeaderData>
Connection<TVector, TTransport, THeaderData>::Connection(
    Connection<TVector, TTransport, THeaderData>&& other) noexcept
    : m_transport(std::move(other.m_transport)) {}

template <typename TVector, typename TTransport, typename THeaderData>
Connection<TVector, TTransport, THeaderData>& Connection<TVector, TTransport, THeaderData>::
operator=(Connection<TVector, TTransport, THeaderData>&& other) noexcept {
  if (&other != this) {
    m_transport = std::move(other.m_transport);
  }
  return *this;
}

template <typename TVector, typename TTransport, typename THeaderData>
Result Connection<TVector, TTransport, THeaderData>::connect(const std::string& address,
                                                    u16 port) {
  return m_transport.connect(address, port);
}

template <typename TVector, typename TTransport, typename THeaderData>
void Connection<TVector, TTransport, THeaderData>::disconnect() {
  m_transport.disconnect();
}

// TODO go over the types used
template <typename TVector, typename TTransport, typename THeaderData>
std::tuple<Result, THeaderData> Connection<TVector, TTransport, THeaderData>::read(
    TVector& payload_out) {
  Header header{};
  ssize_t bytes = 0;
  // TODO make it possible to break out of loops if bad header
  while (bytes < Header::get_header_size()) {
    const auto maybe_bytes =
        m_transport.read(header.get(), Header::get_header_size());
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return std::make_tuple<Result, THeaderData>(Result::kFail, THeaderData{});
    }
  }

  if (payload_out.capacity() < header.get_payload_size()) {
    payload_out.reserve(header.get_payload_size());
  }

  payload_out.resize(payload_out.capacity());
  bytes = 0;
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.get_payload_size()) {
    // TODO timeout read in case bad info in header
    const auto maybe_bytes = m_transport.read(
        &payload_out[bytes], header.get_payload_size() - bytes);
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return std::make_tuple<Result, THeaderData>(Result::kFail,
                                                  header.get_header_data());
    }
  }
  payload_out.resize(static_cast<size_t>(bytes));
  return std::make_tuple<Result, THeaderData>(Result::kSuccess,
                                              header.get_header_data());
}

template <typename TVector, typename TTransport, typename THeaderData>
Result Connection<TVector, TTransport, THeaderData>::write(
    const THeaderData& header_data, const TVector& payload) {
  const auto payload_size = payload.size();
  if (payload_size > std::numeric_limits<typename Header::Payload_size>::max() ||
      payload_size < std::numeric_limits<typename Header::Payload_size>::min()) {
    // TODO send payloads larger than what can fit in a single packet
    DNET_ASSERT(
        payload_size > std::numeric_limits<typename Header::Payload_size>::max() ||
            payload_size < std::numeric_limits<typename Header::Payload_size>::min(),
        "Cannot fit the payload in the packet");
  }
  const Header header{static_cast<typename Header::Payload_size>(payload_size),
                      header_data};

  ssize_t bytes = 0;
  // TODO make it possible to break out of loops if bad header
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.get_header_size()) {
    const auto maybe_bytes =
        m_transport.write(header.get_const(), Header::get_header_size());
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return Result::kFail;
    }
    // TODO not using bytes
  }

  bytes = 0;
  // TODO bad cast
  while (static_cast<size_t>(bytes) < header.get_payload_size()) {
    const auto maybe_bytes =
        m_transport.write(&payload[bytes], header.get_payload_size() - bytes);
    if (maybe_bytes.has_value()) {
      bytes += maybe_bytes.value();
    } else {
      return Result::kFail;
    }
  }

  return Result::kSuccess;
}

template <typename TVector, typename TTransport, typename THeaderData>
bool Connection<TVector, TTransport, THeaderData>::can_read() const {
  return m_transport.can_read();
}

template <typename TVector, typename TTransport, typename THeaderData>
bool Connection<TVector, TTransport, THeaderData>::can_write() const {
  return m_transport.can_write();
}

template <typename TVector, typename TTransport, typename THeaderData>
bool Connection<TVector, TTransport, THeaderData>::can_accept() const {
  return m_transport.can_accept();
}

template <typename TVector, typename TTransport, typename THeaderData>
bool Connection<TVector, TTransport, THeaderData>::has_error() const {
  return m_transport.has_error();
}

template <typename TVector, typename TTransport, typename THeaderData>
Result Connection<TVector, TTransport, THeaderData>::start_server(u16 port) {
  return m_transport.start_server(port);
}

template <typename TVector, typename TTransport, typename THeaderData>
std::optional<Connection<TVector, TTransport, THeaderData>>
Connection<TVector, TTransport, THeaderData>::accept() {
  auto maybe_transport = m_transport.accept();
  if (maybe_transport.has_value()) {
    return std::optional<Connection<TVector, TTransport, THeaderData>>{
        Connection(std::move(maybe_transport.value()))};
  }
  return std::nullopt;
}

}  // namespace dnet

#endif  // CONNECTION_HPP_
