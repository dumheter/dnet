#ifndef NET_HEADER_HPP
#define NET_HEADER_HPP

// ============================================================ //
// Headers
// ============================================================ //

#include <cstddef>
#include <dnet/payload/payload.hpp>

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  enum class Packet_type : u8
  {
    INVALID = 0,
    RESPONSE,
    REQUEST,
    UPDATE,
    INFO,
    ERROR,

    LAST_ENUN
  };

  class Header_interface
  {
  public:

    //virtual ~Header_interface() = 0;

    virtual size_t get_payload_size() const = 0;

    // must be implemented
    //virtual static constexpr size_t get_header_size() const = 0;

    virtual Packet_type get_type() const = 0;

    virtual bool is_valid() = 0;

    virtual void build_header(payload_container& payload) = 0;

    virtual u8* get() = 0;

  private:

    virtual void set_payload_size(size_t size) = 0;

    virtual void set_type(Packet_type type) = 0;
  };

}

#endif //NET_HEADER_HPP
