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

#ifndef NETWORK_EVENT_HPP_
#define NETWORK_EVENT_HPP_

#include <string>

namespace dnet {

class NetworkEvent {
 public:
  enum class Type {
    // there is data to be read
    kNewData = 0,
    // we disconnected
    kDisconnected,
    // connected to a server
    kConnected,
    // unable to send data, since sendQueue was full
    kSendQueueFull,
    // unable to recv data, since recvQueue was full
    kRecvQueueFull,
    // unable to connect to the given address
    kFailedToConnect,

    // used with default constructor
    kInvalid
  };

  NetworkEvent() = default;

  NetworkEvent(Type type) : type_(type) {}

  Type type() const { return type_; }

  void set_type(Type type) { type_ = type; }

  std::string ToString() const {
    std::string str{};
    switch (type_) {
      case Type::kNewData:
        str = "new data";
        break;
      case Type::kDisconnected:
        str = "disconnected";
        break;
      case Type::kConnected:
        str = "connected";
        break;
      case Type::kSendQueueFull:
        str = "send queue full";
        break;
      case Type::kRecvQueueFull:
        str = "recv queue full";
        break;
      case Type::kFailedToConnect:
        str = "failed to connect";
        break;
      case Type::kInvalid:
        str = "invalid";
        break;
    }
    return str;
  }

 private:
  Type type_ = Type::kInvalid;
};

}  // namespace dnet

#endif  // NETWORK_EVENT_HPP_
