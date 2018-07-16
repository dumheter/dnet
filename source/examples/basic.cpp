#include <iostream>
#include <net/transport/tcp.hpp>
#include <net/connection/connection.hpp>

using namespace dnet;

int main()
{
  startup();

  struct Foo
  {
    u8 ayy;
  };

  Foo foo{};
  std::cout << std::to_string(std::numeric_limits<decltype(foo.ayy)>::max()) << "\n";

  shutdown();
  return 0;
}