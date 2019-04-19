#define DOCTEST_CONFIG_IMPLEMENT
#include "dnet/util/util.hpp"
#include "doctest.h"

int main(int argc, char** argv) {
  dnet::Startup();

  doctest::Context ctx;
  ctx.applyCommandLine(argc, argv);
  const int res = ctx.run();

  dnet::Shutdown();
  return res;
}
