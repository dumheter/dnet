#ifndef NET_SOCKET_HPP
#define NET_SOCKET_HPP

// ============================================================ //
// Headers
// ============================================================ //

#define CHIF_NET_IMPLEMENTATION

#include <net/util/dnet_exception.hpp>
#include <net/util/types.hpp>
#include "chif/chif_net.h"

// ====================================================================== //
// Exception Class
// ====================================================================== //

namespace dnet
{

  class socket_exception : public dnet_exception
  {
  public:

    explicit socket_exception(const std::string& what)
      : dnet_exception(what) {};

    explicit socket_exception(const char* what)
      : dnet_exception(what) {};

  };
}

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  class Socket
  {
  public:

    /*
     * @throw socket_exception if unable to open socket
     */
    Socket(chif_net_protocol proto, chif_net_address_family fam);

    ~Socket() { chif_net_close_socket(&m_socket); };

    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;

    Socket(Socket&& other) noexcept ;
    Socket& operator=(Socket&& other) noexcept ;

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
    void bind(u16 port);

    /*
     * @throw socket_exception if unable to listen
     */
    void listen();

    /*
     * @throw socket_exception if unable to accept
     */
    Socket accept();

    /*
     * @throw socket_exception if fail to read
     */
    ssize_t read(u8* buf, size_t len);

    /*
     * @throw socket_exception if fail to read
     */
    ssize_t write(u8* buf, size_t len);

    void close();

    /*
     * @throw socket_exception if unable to connect or create address
     */
    void connect(const std::string& ip, u16 port);

    /*
     * @throw socket_exception
     */
    bool can_write();

    /*
     * @throw socket_exception
     */
    bool can_read();

    /*
     * @throw socket_exception
     */
    std::string get_remote_ip();

    /*
     * @throw socket_exception
     */
    u16 get_remote_port();

    /*
     * @throw socket_exception
     */
    void set_reuse_addr(bool reuse);

    /*
     * @throw socket_exception
     */
    void set_blocking(bool blocking);

  private:
    chif_net_socket m_socket;
    chif_net_protocol m_proto;
    chif_net_address_family m_fam;

  };

}

#endif //NET_SOCKET_HPP
