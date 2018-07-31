#ifndef NET_DNET_EXCEPTION_HPP
#define NET_DNET_EXCEPTION_HPP

// ============================================================ //
// Headers
// ============================================================ //

#include <stdexcept>

// ============================================================ //
// Class Declaration
// ============================================================ //

namespace dnet
{

  class dnet_exception : public std::runtime_error
  {
  public:

    explicit dnet_exception(const std::string& what)
      : std::runtime_error(what) {};

    explicit dnet_exception(const char* what)
      : std::runtime_error(what) {};

  };

}

#endif //NET_DNET_EXCEPTION_HPP
