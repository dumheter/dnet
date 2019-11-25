# dnet - simplify networking
Have you been looking for an easy to use networking library for c++17? You've come to the right place! It has...

* Thin socket wrapper, __socket.hpp__.
* Transport wrappers, __udp.hpp__ & __tcp.hpp__.
* Abstractions, such as __tcp_connection.hpp__ & __network_handler.hpp__.

## Usage NetworkHandler
For more in-depth usage, see __network_handler.test.cpp__.

## Usage TcpConnection
For more in-depth usage, see __tcp_connection.test.cpp__.
minimal working server:
```cpp
dnet::TcpConnection<dnet::Tcp> server{};
server.StartServer(port);

auto maybe_client = server.Accept();
if (maybe_client.has_value()) {
    auto client = maybe_client.value();

    // read from client
    // use "structured binding" to capture variables from tuple
    dnet::payload_container payload{};
    auto [result, header] = client.Read(payload);

    // do stuff with payload
    ...
}
```

minimal working client:
```cpp
dnet::TcpConnection<dnet::Tcp> client{};
client.Connect(ip, port);

dnet::payload_container payload{};
// fill payload with data
...
client.Write(payload);
```

## Usage Tcp
coming soon™

## Usage Udp
coming soon™

## Usage Socket
coming soon™

## Dependencies
dnet uses chif_net which is a cross-platform socket library written in C.
It also uses dutil for the dutil::queue. There are some more optional
dependencies for building the tests and examples.

## Windows / Cross-Platform
If you target windows, make sure to use:
```cpp
// in begining of program
dnet::Startup();

// before ending program
dnet::Shutdown();
```
