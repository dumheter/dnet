#ifndef NET_UTIL_HPP
#define NET_UTIL_HPP

// ============================================================ //
// Headers
// ============================================================ //

#include <thirdparty/chif/chif_net.h>

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  inline void startup() { chif_net_startup(); };

  inline void shutdown() { chif_net_shutdown(); };

}

#endif //NET_UTIL_HPP
