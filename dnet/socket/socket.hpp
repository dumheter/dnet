#ifndef SOCKET_HPP_
#define SOCKET_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include <optional>
#include <string>
#include <tuple>
#include "chif_net/chif_net.h"
#include "dnet/util/result.hpp"
#include "dnet/util/types.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet {

class Socket {
 public:
  Socket(chif_net_protocol proto, chif_net_address_family fam);

  ~Socket() { chif_net_close_socket(&m_socket); };

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
  Result open();

  Result bind(u16 port);

  Result listen();

  std::optional<Socket> accept();

  /**
   * @return Amount of read bytes, or nullopt on failure.
   */
  std::optional<ssize_t> read(u8* buf_out, size_t buflen);

  /**
   * @return Amount of written bytes, or nullopt on failure.
   */
  std::optional<ssize_t> write(const u8* buf, size_t buflen);

  void close();

  Result connect(const std::string& address, u16 port);

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool can_write() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool can_read() const;

  /**
   * @return Any error occured while attempting to check, will return false.
   */
  bool has_error() const;

  /**
   * @return Ip address, or nullopt on failure.
   */
  std::optional<std::string> get_ip();

  /**
   * @return Port, or nullopt on failure.
   */
  std::optional<u16> get_port();

  /**
   * @return Result of the call, Ip and port of peer.
   */
  std::tuple<Result, std::string, u16> get_peer();

  Result set_reuse_addr(bool reuse) const;

  Result set_blocking(bool blocking) const;

  std::string last_error_to_string() const;

 private:
  chif_net_socket m_socket;
  chif_net_protocol m_proto;
  chif_net_address_family m_fam;
  chif_net_result m_last_error;
};

}  // namespace dnet

#endif  // SOCKET_HPP_
