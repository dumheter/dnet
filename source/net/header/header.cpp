// ============================================================ //
// Headers
// ============================================================ //

#include <cstring>
#include <limits>
#include <stdexcept>
#include "header.hpp"

// ============================================================ //
// Class Definition
// ============================================================ //

namespace dnet
{

  Header::Header()
    : m_header(new u8[sizeof(Header_meta)])
  {

  }

  Header::Header(const Header& other)
  : m_header(new u8[sizeof(Header_meta)])
  {
    static_assert(sizeof(u8) == sizeof(unsigned char),
                  "memcpy reqires both pointer to be of same type");
    memcpy(m_header, other.m_header, sizeof(Header_meta));
  }

  Header& Header::operator=(const Header& other)
  {
    memcpy(m_header, other.m_header, sizeof(Header_meta));

    return *this;
  }

  Header::Header(Header&& other) noexcept
  {
    m_header = other.m_header;
    other.m_header = nullptr;
  }

  Header& Header::operator=(Header&& other) noexcept
  {
    if (&other != this) {
      m_header = other.m_header;
      other.m_header = nullptr;
    }

    return *this;
  }

  Header::~Header()
  {
    delete[] m_header;
  }

  size_t Header::get_payload_size() const
  {
    Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
    return header->size;
  }

  Packet_type Header::get_type() const
  {
    Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
    return header->type;
  }

  bool Header::is_valid()
  {
    Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
    return (header->type != Packet_type::INVALID &&
            header->type < Packet_type::LAST_ENUN);
  }

  void Header::set_payload_size(size_t size)
  {
     Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
     if (size > std::numeric_limits<decltype(header->size)>::max())
       throw std::runtime_error("size too large, narrowing would occur");
     header->size = size;
  }

  void Header::set_type(Packet_type type)
  {
    Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
    header->type = type;
  }

  void Header::build_header(payload_container& payload)
  {
    set_payload_size(payload.size());
    set_type(Packet_type::INFO);
  }

  u8* Header::get()
  {
    return m_header;
  }

}
