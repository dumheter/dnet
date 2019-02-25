#ifndef SOCKET_HPP_
#define SOCKET_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include "chif_net/chif_net.h"
#include "dnet/util/dnet_exception.hpp"
#include "dnet/util/types.hpp"

// ====================================================================== //
// Exception Class
// ====================================================================== //

namespace dnet {

class socket_exception : public dnet_exception {
 public:
  explicit socket_exception(const std::string& what) : dnet_exception(what){};

  explicit socket_exception(const char* what) : dnet_exception(what){};
};
}  // namespace dnet

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet {

class Socket {
 public:
  /*
   * @throw socket_exception if unable to open socket
   */
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
  /*
   * @throw socket_exception if unable to bind
   */
  void bind(u16 port) const;

  /*
   * @throw socket_exception if unable to listen
   */
  void listen() const;

  /*
   * @throw socket_exception if unable to accept
   */
  Socket accept() const;

  /*
   * @throw socket_exception if fail to read
   */
  ssize_t read(u8* buf_out, size_t len) const;

  /*
   * @throw socket_exception if fail to read
   */
  ssize_t write(const u8* buf, size_t len) const;

  void close();

  /*
   * @throw socket_exception if unable to connect or create address
   */
  void connect(const std::string& ip, u16 port);

  /*
   * @throw socket_exception
   */
  bool can_write() const;

  /*
   * @throw socket_exception
   */
  bool can_read() const;

  bool has_error() const;

  /*
   * @throw socket_exception
   */
  std::string get_ip() const;

  /*
   * @throw socket_exception
   */
  u16 get_port() const;

  /*
   * @throw socket_exception
   */
  std::string get_remote_ip() const;

  /*
   * @throw socket_exception
   */
  u16 get_remote_port() const;

  /*
   * @throw socket_exception
   */
  void set_reuse_addr(bool reuse) const;

  /*
   * @throw socket_exception
   */
  void set_blocking(bool blocking) const;

 private:
  chif_net_socket m_socket;
  chif_net_protocol m_proto;
  chif_net_address_family m_fam;
};

}  // namespace dnet

#endif  // SOCKET_HPP_
