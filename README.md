# dnet - simplify networking
Have you been looking for an easy to use networking library for c++17? You've come to the right place!

## Usage
minimal working server:
```cpp
dnet::Connection<dnet::Tcp> server{};
server.start_server(port);

auto maybe_client = server.accept();
if (maybe_client.has_value()) {
    auto client = std::move(maybe_client.value());

    // read from client
    // use "structured binding" to capture variables from tuple
    dnet::payload_container payload{};
    auto [result, header] = client.read(payload);

    // do stuff with payload
    ...
}
```

minimal working client:
```cpp
dnet::Connection<dnet::Tcp> client{};
client.connect(ip, port);

dnet::payload_container payload{};
// fill payload with data
...
client.write(payload);
```

## Custom header
For now, look at custom_header_data.cpp in examples.

## Dependencies
dnet uses chif_net which is a cross-platform socket library written in C.

## Windows
If you target windows, make sure to use:
```cpp
// in begining of program
dnet::startup();

// before ending program
dnet::shutdown();
```
