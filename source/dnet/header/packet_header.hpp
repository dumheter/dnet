#ifndef NET_HEADER_SMALL_HPP
#define NET_HEADER_SMALL_HPP

// ============================================================ //
// Headers
// ============================================================ //

#include <dnet/util/types.hpp>

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

  class Packet_header
  {
  public:
    Packet_header();

    Packet_header(const Packet_header& other);
    Packet_header& operator=(const Packet_header& other);

    Packet_header(Packet_header&& other) noexcept;
    Packet_header& operator=(Packet_header&& other) noexcept;

    ~Packet_header();

    size_t get_payload_size() const;

    static constexpr size_t get_header_size() { return sizeof(Header_meta); }

    Packet_type get_type() const;

    bool is_valid();

    void build_header(size_t payload_size);

    u8* get();

  private:
    void set_payload_size(size_t size);

    void set_type(Packet_type type);

  public:
    struct Header_meta
    {
      Packet_type type;
      u16 size;
    };

  private:
    u8* m_header;

  };

}

#endif //NET_HEADER_SMALL_HPP
