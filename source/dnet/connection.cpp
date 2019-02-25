#include "connection.hpp"

namespace dnet
{

connection_exception::connection_exception(const std::string& what)
    : dnet_exception(what) {}

connection_exception::connection_exception(const char* what)
    : dnet_exception(what) {}

}
