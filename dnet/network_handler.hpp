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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NETWORK_HANDLER_HPP_
#define NETWORK_HANDLER_HPP_

#include <chrono>
#include <dnet/net/network_event.hpp>
#include <dnet/util/dnet_assert.hpp>
#include <dnet/util/result.hpp>
#include <dnet/util/types.hpp>
#include <dutil/queue.hpp>
#include <optional>
#include <string>
#include <thread>

namespace dnet {

/**
 * Structure used to communicate between worker thread and main thread.
 */
template <typename TPacket>
struct SharedData {
  dutil::Queue<TPacket> send_queue{512};
  dutil::Queue<TPacket> recv_queue{512};
  dutil::Queue<NetworkEvent> eventQueue{512};
  bool run_worker_thread_flag = true;
  bool connect_flag = false;
  bool is_connected = false;
  std::string ip{""};
  u16 port = 0;

  SharedData() = default;

  SharedData(const SharedData& other) = delete;
  SharedData& operator=(const SharedData& other) = delete;

  SharedData(SharedData&& other) noexcept = default;
  SharedData& operator=(SharedData&& other) noexcept = default;
};

/**
 * The function that will run on the worker thread.
 */
namespace network_worker {
template <typename TPacket, typename TTransport>
void Loop(SharedData<TPacket>& shared_data);
}  // namespace network_worker

// ============================================================ //
// NetworkHandler declaration
// ============================================================ //

/**
 * Thread safe network handler
 *
 * @tparam TPacket The packet type that will be sent to
 */
template <typename TPacket, typename TTransport>
class NetworkHandler {
 public:
  NetworkHandler();

  // no copy
  NetworkHandler(const NetworkHandler& other) = delete;
  NetworkHandler& operator=(const NetworkHandler& other) = delete;

  NetworkHandler(NetworkHandler&& other) noexcept;
  NetworkHandler& operator=(NetworkHandler&& other) noexcept;

  ~NetworkHandler();

  /**
   * @param data The data to be sent.
   * @return If the data was successfully queued for sending.
   */
  bool Send(const TPacket& packet);
  bool Send(TPacket&& packet);

  /**
   * Retrieve the first data from the queue. Call hasData before calling this.
   * @return Data Will return the first data element in queue. If nothing
   * is there, NULL_DATA will be returned.
   */
  std::optional<TPacket> Recv();

  bool HasEvent();

  NetworkEvent GetEvent();

  /**
   * Connect to a remote device
   */
  void Connect(const std::string& ip, u16 port);

  bool is_connected();

  const SharedData<TPacket>& GetSharedData();

 private:
  SharedData<TPacket> shared_data_{};
  std::thread worker_;
};

// ============================================================ //
// NetworkHandler template definition
// ============================================================ //

template <typename TPacket, typename TTransport>
NetworkHandler<TPacket, TTransport>::NetworkHandler()
    : worker_(network_worker::Loop<TPacket, TTransport>,
              std::ref(shared_data_)) {}

template <typename TPacket, typename TTransport>
NetworkHandler<TPacket, TTransport>::NetworkHandler(
    NetworkHandler&& other) noexcept
    : shared_data_(std::move(other.shared_data_)), worker_() {
  worker_.swap(other.worker_);
}

template <typename TPacket, typename TTransport>
NetworkHandler<TPacket, TTransport>& NetworkHandler<TPacket, TTransport>::
operator=(NetworkHandler&& other) noexcept {
  if (this != &other) {
    shared_data_ = std::move(other.shared_data_);
    worker_ = std::move(other.worker_);
  }
  return *this;
}

template <typename TPacket, typename TTransport>
NetworkHandler<TPacket, TTransport>::~NetworkHandler() {
  shared_data_.run_worker_thread_flag = false;
  worker_.join();
}

template <typename TPacket, typename TTransport>
bool NetworkHandler<TPacket, TTransport>::Send(const TPacket& packet) {
  const dutil::QueueResult res = shared_data_.send_queue.Push(packet);
  return res == dutil::QueueResult::kSuccess;
}

// TODO perfect forwarding
template <typename TPacket, typename TTransport>
bool NetworkHandler<TPacket, TTransport>::Send(TPacket&& packet) {
  const dutil::QueueResult res =
      shared_data_.send_queue.Push(std::move(packet));
  return res == dutil::QueueResult::kSuccess;
}

template <typename TPacket, typename TTransport>
std::optional<TPacket> NetworkHandler<TPacket, TTransport>::Recv() {
  TPacket packet;
  const dutil::QueueResult res = shared_data_.recv_queue.Pop(packet);
  if (res == dutil::QueueResult::kSuccess) {
    return std::optional<TPacket>(std::move(packet));
  }
  return std::nullopt;
}

template <typename TPacket, typename TTransport>
bool NetworkHandler<TPacket, TTransport>::HasEvent() {
  return !shared_data_.eventQueue.Empty();
}

template <typename TPacket, typename TTransport>
NetworkEvent NetworkHandler<TPacket, TTransport>::GetEvent() {
  // TODO have event be returned instead of output param
  NetworkEvent event{
      NetworkEvent::Type::kNewData};  // will be overwritten next line
  // TODO how to handle error from eventQueue?
  shared_data_.eventQueue.Pop(event);
  return event;
}

template <typename TPacket, typename TTransport>
void NetworkHandler<TPacket, TTransport>::Connect(const std::string& ip,
                                                  u16 port) {
  if (!shared_data_.connect_flag) {
    shared_data_.ip = ip;
    shared_data_.port = port;
    shared_data_.connect_flag = true;
  }
}

template <typename TPacket, typename TTransport>
bool NetworkHandler<TPacket, TTransport>::is_connected() {
  return shared_data_.is_connected;
}

template <typename TPacket, typename TTransport>
const SharedData<TPacket>&
NetworkHandler<TPacket, TTransport>::GetSharedData() {
  return shared_data_;
}

}  // namespace dnet

// ============================================================ //
// Worker Thread declaration
// ============================================================ //

namespace dnet::network_worker {
// TODO right now, it expects a TTransport, but it should really operate on
// a connection instead. For example the write and read cannot dynamically
// allocate more memory, which requires you to always allocate the max
// size a packet could be.
// TODO when adding NetworkEvent, we dont check for errors on the eventQueue
/**
 * The network handler worker thread code.
 *
 * @param TPacket see network handler for info.
 * @param TTransport The transport type that will send the data. Must
 * follow the dnet transport api.
 */
template <typename TPacket, typename TTransport>
class Worker {
 public:
  Worker() = default;

  // no copy
  Worker(const Worker& other) = delete;
  Worker& operator=(const Worker& other) = delete;

  ~Worker() = default;

  void Disconnect(dutil::Queue<NetworkEvent>& eventQueue, bool& is_connected) {
    const dutil::QueueResult queueResult =
        eventQueue.Push(NetworkEvent(NetworkEvent::Type::kDisconnected));
    dnet_assert(queueResult == dutil::QueueResult::kSuccess,
                "failed to push network event to eventQueue");
    transport_.Disconnect();
    is_connected = false;
  }

  void HandleSend(dutil::Queue<NetworkEvent>& eventQueue,
                  dutil::Queue<TPacket>& send_queue, bool& is_connected) {
    TPacket packet{};
    dutil::QueueResult queueResult = send_queue.Pop(packet);
    if (queueResult == dutil::QueueResult::kSuccess) {
      const auto maybe_bytes = transport_.Write(packet.data(), packet.size());
      if (!maybe_bytes.has_value() ||
          maybe_bytes.value() != static_cast<ssize_t>(packet.size())) {
        // TODO send the error information with the event?
        Disconnect(eventQueue, is_connected);
      }
    } else {
      queueResult =
          eventQueue.Push(NetworkEvent(NetworkEvent::Type::kSendQueueFull));
      dnet_assert(queueResult == dutil::QueueResult::kSuccess,
                  "failed to push network event to eventQueue");
    }
  }

  bool CanRecv() const { return transport_.CanRead(); }

  void HandleCanRecv(dutil::Queue<NetworkEvent>& eventQueue,
                     dutil::Queue<TPacket>& recv_queue, bool& is_connected) {
    TPacket packet(std::numeric_limits<u16>::max());
    const auto maybe_bytes = transport_.Read(packet.data(), packet.capacity());
    if (maybe_bytes.has_value()) {
      packet.resize(maybe_bytes.value());
    }
    if (maybe_bytes.has_value() &&
        maybe_bytes.value() == static_cast<ssize_t>(packet.size())) {
      dutil::QueueResult queueResult;
      queueResult = recv_queue.Push(std::move(packet));
      if (queueResult == dutil::QueueResult::kSuccess) {
        queueResult =
            eventQueue.Push(NetworkEvent(NetworkEvent::Type::kNewData));
        dnet_assert(queueResult == dutil::QueueResult::kSuccess,
                    "failed to push network event to eventQueue");
      } else {
        // TODO can other errors happen?
        queueResult =
            eventQueue.Push(NetworkEvent(NetworkEvent::Type::kRecvQueueFull));
        dnet_assert(queueResult == dutil::QueueResult::kSuccess,
                    "failed to push network event to eventQueue");
      }

    } else {
      // TODO is dropping the client the right thing to do after failed read?
      // TODO send the error information with the event?
      Disconnect(eventQueue, is_connected);
    }
  }

  void addConnection(dutil::Queue<NetworkEvent>& eventQueue,
                     const std::string& ip, u16 port, bool& is_connected) {
    // make sure old connection is closed
    transport_.Disconnect();

    const auto res = transport_.Connect(ip, port);
    dutil::QueueResult queueResult;
    if (res == Result::kSuccess) {
      // TODO send the information with the event?
      queueResult =
          eventQueue.Push(NetworkEvent(NetworkEvent::Type::kConnected));
      dnet_assert(queueResult == dutil::QueueResult::kSuccess,
                  "failed to push network event to eventQueue");
      is_connected = true;
    } else {
      // TODO send the error information with the event?
      queueResult =
          eventQueue.Push(NetworkEvent(NetworkEvent::Type::kFailedToConnect));
      dnet_assert(queueResult == dutil::QueueResult::kSuccess,
                  "failed to push network event to eventQueue");
    }
  }

  /**
   * Explicitly check if the connection is open. Note, best way to check for a
   * broken connection is to acually send data and expect a response.
   */
  bool isClientConnected() const { return transport_.CanWrite(); }

 private:
  TTransport transport_{};
};

/**
 * This is the loop the network worker thread will be running.
 * @param shared_data This is used to communicate between main thread and
 * worker.
 */
template <typename TPacket, typename TTransport>
void Loop(SharedData<TPacket>& shared_data) {
  Worker<TPacket, TTransport> worker{};

  bool did_work;
  while (shared_data.run_worker_thread_flag) {
    did_work = false;
    if (worker.isClientConnected()) {
      if (!shared_data.send_queue.Empty()) {
        did_work = true;
        worker.HandleSend(shared_data.eventQueue, shared_data.send_queue,
                          shared_data.is_connected);
      }

      if (worker.CanRecv()) {
        did_work = true;
        worker.HandleCanRecv(shared_data.eventQueue, shared_data.recv_queue,
                             shared_data.is_connected);
      }
    }

    if (shared_data.connect_flag) {
      did_work = true;
      worker.addConnection(shared_data.eventQueue, shared_data.ip,
                           shared_data.port, shared_data.is_connected);
      shared_data.connect_flag = false;
    }

    // to not use 100% CPU, let it sleeep every loop
    if (!did_work) {
      constexpr std::chrono::microseconds sleep_time_us{100};
      std::this_thread::sleep_for(sleep_time_us);
    }
  }
}

}  // namespace dnet::network_worker

#endif  // NETWORK_HANDLER_HPP_
