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
