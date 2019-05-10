#ifndef NETWORK_EVENT_HPP_
#define NETWORK_EVENT_HPP_

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

 private:
  Type type_ = Type::kInvalid;
};

}  // namespace dnet

#endif  // NETWORK_EVENT_HPP_
