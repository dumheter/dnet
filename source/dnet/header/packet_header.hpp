#ifndef NET_HEADER_SMALL_HPP
#define NET_HEADER_SMALL_HPP

// ============================================================ //
// Headers
// ============================================================ //

#include <dnet/util/types.hpp>
#include "header_interface.hpp"

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  class Packet_header : public Header_interface
  {

  public:

    Packet_header();

    Packet_header(const Packet_header& other);
    Packet_header& operator=(const Packet_header& other);

    Packet_header(Packet_header&& other) noexcept;
    Packet_header& operator=(Packet_header&& other) noexcept;

    ~Packet_header();

    size_t get_payload_size() const override;

    static constexpr size_t get_header_size() { return sizeof(Header_meta); };

    Packet_type get_type() const override;

    bool is_valid() override;

    void build_header(payload_container& payload) override;

    u8* get() override;

  private:

    void set_payload_size(size_t size) override;

    void set_type(Packet_type type) override;

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
