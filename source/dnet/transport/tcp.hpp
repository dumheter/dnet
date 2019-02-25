#ifndef TCP_HPP_
#define TCP_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include <string>
#include "dnet/socket/socket.hpp"
#include "dnet/util/types.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet {

class Tcp {
 public:
  Tcp();

  Tcp(const Tcp& other) = delete;
  Tcp& operator=(const Tcp& other) = delete;

  Tcp(Tcp&& other) noexcept;
  Tcp& operator=(Tcp&& other) noexcept;

 private:
  explicit Tcp(Socket&& socket);

 public:
  /*
   * @throw socket_exception
   */
  void start_server(u16 port) const;

  /*
   * block until a client connects
   * @throw socket_exception
   */
  Tcp accept() const;

  /*
   * @throw socket_exception
   */
  void connect(const std::string& ip, u16 port);

  void disconnect();

  inline ssize_t read(u8* buf_out, size_t len) const {
    return m_socket.read(buf_out, len);
  };

  inline ssize_t write(const u8* buf, size_t len) const {
    return m_socket.write(buf, len);
  };

  inline bool can_write() const { return m_socket.can_write(); }

  inline bool can_read() const { return m_socket.can_read(); }

  inline bool can_accept() const { return m_socket.can_read(); }

  inline bool has_error() const { return m_socket.has_error(); }

  inline std::string get_ip() const { return m_socket.get_ip(); }

  inline u16 get_port() const { return m_socket.get_port(); }

  inline std::string get_remote_ip() const { return m_socket.get_remote_ip(); };

  inline u16 get_remote_port() const { return m_socket.get_remote_port(); }

 private:
  Socket m_socket;
};

}  // namespace dnet

#endif  // TCP_HPP_
