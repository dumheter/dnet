/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PACKET_HEADER_HPP_
#define PACKET_HEADER_HPP_

#include <dnet/util/types.hpp>

namespace dnet {

/**
 * You should make you own header data struct.
 */
struct HeaderDataExample {
  u32 type = 1337;
};

/**
 * @tparam THeaderData Will be put in the header after header size.
 */
template <typename THeaderData>
class PacketHeader {
 public:
  using PayloadSize = u32;

  struct PayloadInfo {
    PayloadSize payload_size = 0;
    THeaderData header_data{};
  };

  PacketHeader() = default;

  explicit PacketHeader(PayloadSize payload_size, THeaderData header_data)
      : header_({payload_size, header_data}) {}

  PayloadSize payload_size() const { return header_.payload_size; }

  void set_payload_size(PayloadSize payload_size) {
    header_.payload_size = payload_size;
  }

  THeaderData header_data() const { return header_.header_data; }

  void set_header_data(THeaderData header_data) {
    header_.header_data = header_data;
  }

  static constexpr PayloadSize header_size() {
    return sizeof(PayloadInfo);
  }

  u8* get() { return reinterpret_cast<u8*>(&header_); }

  const u8* get() const { return reinterpret_cast<const u8*>(&header_); }

 private:
  PayloadInfo header_;
};

}  // namespace dnet

#endif  // PACKET_HEADER_HPP_
