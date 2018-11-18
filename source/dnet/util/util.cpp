#include "util.hpp"
#include "platform.hpp"
#include <sys/resource.h>
#include <cassert>

namespace dnet
{
  bool set_nofd_soft_limit(int limit)
  {
#if defined(DNET_PLATFORM_WINDOWS)
    assert(!"not implemented");
    return false;
#endif
    struct rlimit rlp;
    auto res = getrlimit(RLIMIT_NOFILE, &rlp);
    if (res != 0) return false;

    rlp.rlim_cur = limit == NOFD_HARD_LIMIT ? rlp.rlim_max : limit;
    res = setrlimit(RLIMIT_NOFILE, &rlp);
    return res == 0;
  }

  int get_nofd_soft_limit()
  {
#if defined(DNET_PLATFORM_WINDOWS)
    assert(!"not implemented");
    return -1;
#endif
    struct rlimit rlp;
    auto res = getrlimit(RLIMIT_NOFILE, &rlp);
    if (res != 0) return -1;
    else return rlp.rlim_cur;
  }

  int get_nofd_hard_limit()
  {
#if defined(DNET_PLATFORM_WINDOWS)
    assert(!"not implemented");
    return -1;
#endif
    struct rlimit rlp;
    auto res = getrlimit(RLIMIT_NOFILE, &rlp);
    if (res != 0) return -1;
    else return rlp.rlim_max;
  }
}
