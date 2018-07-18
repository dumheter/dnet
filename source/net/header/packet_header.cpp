// ============================================================ //
// Headers
// ============================================================ //

#include <cstring>
#include <limits>
#include <stdexcept>
#include "packet_header.hpp"

// ============================================================ //
// Class Definition
// ============================================================ //

namespace dnet
{

  Packet_header::Packet_header()
    : m_header(new u8[sizeof(Header_meta)])
  {

  }

  Packet_header::Packet_header(const Packet_header& other)
  : m_header(new u8[sizeof(Header_meta)])
  {
    static_assert(sizeof(u8) == sizeof(unsigned char),
                  "memcpy reqires both pointer to be of same type");
    memcpy(m_header, other.m_header, sizeof(Header_meta));
  }

  Packet_header& Packet_header::operator=(const Packet_header& other)
  {
    if (&other != this) {
      if (m_header)
        delete[] m_header;

      m_header = new u8[sizeof(Header_meta)];
      memcpy(m_header, other.m_header, sizeof(Header_meta));
    }

    return *this;
  }

  Packet_header::Packet_header(Packet_header&& other) noexcept
  {
    m_header = other.m_header;
    other.m_header = nullptr;
  }

  Packet_header& Packet_header::operator=(Packet_header&& other) noexcept
  {
    if (&other != this) {
      m_header = other.m_header;
      other.m_header = nullptr;
    }

    return *this;
  }

  Packet_header::~Packet_header()
  {
    if (m_header)
      delete[] m_header;
  }

  size_t Packet_header::get_payload_size() const
  {
    Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
    return header->size;
  }

  Packet_type Packet_header::get_type() const
  {
    Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
    return header->type;
  }

  bool Packet_header::is_valid()
  {
    Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
    return (header->type != Packet_type::INVALID &&
            header->type < Packet_type::LAST_ENUN);
  }

  void Packet_header::set_payload_size(size_t size)
  {
     Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
     if (size > std::numeric_limits<decltype(header->size)>::max())
       throw std::runtime_error("size too large, narrowing would occur");
     header->size = size;
  }

  void Packet_header::set_type(Packet_type type)
  {
    Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
    header->type = type;
  }

  void Packet_header::build_header(payload_container& payload)
  {
    set_payload_size(payload.size());
    set_type(Packet_type::INFO);
  }

  u8* Packet_header::get()
  {
    return m_header;
  }

}
