#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include "dnet/util/util.hpp"

int main(int argc, char** argv) {
  dnet::startup();

  doctest::Context ctx;
  ctx.applyCommandLine(argc, argv);
  const int res = ctx.run();

  dnet::shutdown();
  return res;
}
