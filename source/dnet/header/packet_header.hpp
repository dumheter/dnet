#ifndef PACKET_HEADER_HPP_
#define PACKET_HEADER_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include "dnet/util/types.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  class Packet_header
  {
  public:
      struct Header_meta
      {
          u64 size;
      };

  public:
    Packet_header();

    Packet_header(const Packet_header& other);
    Packet_header& operator=(const Packet_header& other);

    Packet_header(Packet_header&& other) noexcept;
    Packet_header& operator=(Packet_header&& other) noexcept;

    ~Packet_header();

    size_t get_payload_size() const;

    static constexpr size_t get_header_size() { return sizeof(Header_meta); }

    void build_header(size_t payload_size);

    u8* get();

    const u8* get_const();

  private:
    void set_payload_size(size_t size);

  private:
    u8* m_header;

  };

}

#endif //PACKET_HEADER_HPP_
