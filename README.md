# dnet - simplify networking
Have you been looking for an easy to use networking library for c++17? You've come to the right place!

## Usage
basic server:
```cpp
// todo
dnet::Connection<dnet::Tcp, dnet::Header> server;
```

## Built with JSON in mind
dnet is perfect to use with nlohmann's json library:
 ```c++
 dnet::Connection<dnet::Tcp, dnet::Header> con;
 fmt::printf("connecting...\n");
 con.connect("127.0.0.1", 1337);
 
 nlohmann::json msg = {
   {"fruit",  "apple"},
   {"number", 8}
 };
 
 std::vector<u8> packed_msg;
 nlohmann::json::to_cbor(msg, packed_msg);
 fmt::printf("sending json packet:\n%s\n", msg.dump(2));
 con.write(packed_msg);
 
 fmt::printf("waiting for response\n");
 con.read(packed_msg);
 msg = nlohmann::json::from_cbor(packed_msg);
 fmt::printf("got response:\n%s\n", msg.dump(2));
 ```