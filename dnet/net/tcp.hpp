#ifndef TCP_HPP_
#define TCP_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include <optional>
#include <string>
#include <tuple>
#include "dnet/net/socket.hpp"
#include "dnet/util/result.hpp"
#include "dnet/util/types.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

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
    return m_socket.connect(address, port);
  }

  void disconnect() { m_socket.close(); }

  std::optional<ssize_t> read(u8* buf_out, size_t buflen) {
    return m_socket.read(buf_out, buflen);
  };

  std::optional<ssize_t> write(const u8* buf, size_t buflen) {
    return m_socket.write(buf, buflen);
  };

  bool can_write() const { return m_socket.can_write(); }

  bool can_read() const { return m_socket.can_read(); }

  bool can_accept() const { return m_socket.can_read(); }

  bool has_error() const { return m_socket.has_error(); }

  std::optional<std::string> get_ip() { return m_socket.get_ip(); }

  std::optional<u16> get_port() { return m_socket.get_port(); }

  /**
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> get_peer() {
    return m_socket.get_peer();
  }

  /**
   * If a Result comes back as kFail, or std::optional as std::nullopt, an
   * error will be set. Use this function to access the error string.
   */
  std::string last_error_to_string() const {
    return m_socket.last_error_to_string();
  }

 private:
  Socket m_socket;
};

}  // namespace dnet

#endif  // TCP_HPP_
