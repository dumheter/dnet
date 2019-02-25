// ============================================================ //
// Headers
// ============================================================ //

#include "packet_header.hpp"
#include <cstring>
#include <limits>
#include "dnet/util/dnet_exception.hpp"

// ============================================================ //
// Class Definition
// ============================================================ //

namespace dnet {

Packet_header::Packet_header() : m_header(new u8[sizeof(Header_meta)]) {
  memset(m_header, 0, sizeof(Header_meta));
}

Packet_header::Packet_header(const Packet_header& other)
    : m_header(new u8[sizeof(Header_meta)]) {
  memcpy(m_header, other.m_header, sizeof(Header_meta));
}

Packet_header& Packet_header::operator=(const Packet_header& other) {
  if (&other != this) {
    delete[] m_header;
    m_header = new u8[sizeof(Header_meta)];
    memcpy(m_header, other.m_header, sizeof(Header_meta));
  }

  return *this;
}

Packet_header::Packet_header(Packet_header&& other) noexcept {
  m_header = other.m_header;
  other.m_header = nullptr;
}

Packet_header& Packet_header::operator=(Packet_header&& other) noexcept {
  if (&other != this) {
    delete[] m_header;
    m_header = other.m_header;
    other.m_header = nullptr;
  }

  return *this;
}

Packet_header::~Packet_header() { delete[] m_header; }

size_t Packet_header::get_payload_size() const {
  Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
  return header->size;
}

void Packet_header::set_payload_size(size_t size) {
  Header_meta* header = reinterpret_cast<Header_meta*>(m_header);
  if (size > std::numeric_limits<decltype(header->size)>::max())
    throw dnet_exception("size too large, narrowing would occur");
  header->size = size;
}

void Packet_header::build_header(size_t payload_size) {
  set_payload_size(payload_size);
}

u8* Packet_header::get() { return m_header; }

const u8* Packet_header::get_const() { return m_header; }

}  // namespace dnet
