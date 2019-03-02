#include "util.hpp"
#include <cassert>
#include "platform.hpp"
#if !defined(DNET_PLATFORM_WINDOWS)
# include <sys/resource.h>
#endif

namespace dnet {

bool set_nofd_soft_limit(int limit) {
#if defined(DNET_PLATFORM_WINDOWS)
  (void)limit;
  assert(!"not implemented");
  return false;
#else
  struct rlimit rlp;
  auto res = getrlimit(RLIMIT_NOFILE, &rlp);
  if (res != 0) return false;

  rlp.rlim_cur = limit == NOFD_HARD_LIMIT ? rlp.rlim_max : limit;
  res = setrlimit(RLIMIT_NOFILE, &rlp);
  return res == 0;
#endif
}

int get_nofd_soft_limit() {
#if defined(DNET_PLATFORM_WINDOWS)
  assert(!"not implemented");
  return -1;
#else
  struct rlimit rlp;
  auto res = getrlimit(RLIMIT_NOFILE, &rlp);
  if (res != 0)
    return -1;
  else
    return rlp.rlim_cur;
#endif
}

int get_nofd_hard_limit() {
#if defined(DNET_PLATFORM_WINDOWS)
  assert(!"not implemented");
  return -1;
#else
  struct rlimit rlp;
  auto res = getrlimit(RLIMIT_NOFILE, &rlp);
  if (res != 0)
    return -1;
  else
    return rlp.rlim_max;
#endif
}

}  // namespace dnet
