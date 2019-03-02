#ifndef PACKET_HEADER_HPP_
#define PACKET_HEADER_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include <cstddef>
#include "dnet/util/types.hpp"
#include "dnet/util/result.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet {


/**
 * TODO make it Packet_header<TPacketType>, where TPacketType is a
 * value that will be packed together with size in the Header_meta.
 * This allows the user of Connection to easily define a custom
 * protocol for a Connection.
 *
 * TODO Packet_header has alot of reinterpret casts, lets not have that.
 */

class Packet_header {
 public:
  struct Header_meta {
    u32 size;
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

  Result build_header(size_t payload_size);

  u8* get() { return m_header; }

  const u8* get_const() const { return m_header; }

 private:
  Result set_payload_size(size_t size);

 private:
  u8* m_header;
};

}  // namespace dnet

#endif  // PACKET_HEADER_HPP_
