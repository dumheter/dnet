#ifndef PACKET_HEADER_HPP_
#define PACKET_HEADER_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include "dnet/util/types.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet {

/**
 * You should make you own header data struct.
 */
struct Header_data_example {
  u32 type = 1337;
};

/**
 * @tparam THeaderData Will be put in the header after header size.
 */
template <typename THeaderData>
class Packet_header {
 public:

  using Payload_size = u32;

  struct Payload_info {
    Payload_size payload_size = 0;
    THeaderData header_data{};
  };

  Packet_header() = default;

  explicit Packet_header(Payload_size payload_size, THeaderData header_data)
      : m_header({payload_size, header_data}) {}

  Payload_size get_payload_size() const { return m_header.payload_size; }

  void set_payload_size(Payload_size payload_size) {
    m_header.payload_size = payload_size;
  }

  THeaderData get_header_data() const { return m_header.header_data; }

  void set_header_data(THeaderData header_data) { m_header.header_data = header_data; }

  static constexpr Payload_size get_header_size() { return sizeof(Payload_info); }

  u8* get() { return reinterpret_cast<u8*>(&m_header); }

  const u8* get_const() const { return reinterpret_cast<const u8*>(&m_header); }

 private:
  Payload_info m_header;
};

}  // namespace dnet

#endif  // PACKET_HEADER_HPP_
