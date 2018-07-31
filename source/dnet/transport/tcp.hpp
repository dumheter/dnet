#ifndef NET_TCP_HPP
#define NET_TCP_HPP

// ============================================================ //
// Headers
// ============================================================ //

#include <dnet/util/types.hpp>
#include <dnet/socket/socket.hpp>

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  class Tcp
  {
  public:

    Tcp();

    Tcp(const Tcp& other) = delete;
    Tcp& operator=(const Tcp& other) = delete;

    Tcp(Tcp&& other) noexcept ;
    Tcp& operator=(Tcp&& other) noexcept;

  private:

    explicit Tcp(Socket&& socket);

  public:

    /*
     * @throw socket_exception
     */
    void start_server(u16 port);

    /*
     * block until a client connects
     * @throw socket_exception
     */
    Tcp accept();

    /*
     * @throw socket_exception
     */
    void connect(const std::string& ip, u16 port);

    inline ssize_t read(u8* buf, size_t len) { return m_socket.read(buf, len); };

    inline ssize_t write(u8* buf, size_t len) { return m_socket.write(buf, len); };

    inline std::string get_ip() { return m_socket.get_ip(); };

    inline u16 get_port() { return m_socket.get_port(); };

    inline std::string get_remote_ip() { return m_socket.get_remote_ip(); };

    inline u16 get_remote_port() { return m_socket.get_remote_port(); };

  private:
    Socket m_socket;

  };

}

#endif //NET_TCP_HPP
