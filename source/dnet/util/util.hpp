#ifndef UTIL_HPP_
#define UTIL_HPP_

// ============================================================ //
// Headers
// ============================================================ //

#include "chif_net/chif_net.h"

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  inline void startup() { chif_net_startup(); };

  inline void shutdown() { chif_net_shutdown(); };

  /**
   * There is a soft and a hard limit to how many file descriptors you can
   * have open. This function allows you to change the soft limit.
   * @limit The new value, use NOFD_HARD_LIMIT to raise the soft limit to
   * be equal to hard limit.
   * @return If it was successful or not.
   */
  bool set_nofd_soft_limit(int limit);

  /**
   * @return the soft limit, or -1 if error.
   */
  int get_nofd_soft_limit();

  /**
   * @return the hard limit, or -1 if error.
   */
  int get_nofd_hard_limit();

  constexpr int NOFD_HARD_LIMIT = -1;

}

#endif //UTIL_HPP_
