#ifndef DNET_ASSERT_HPP_
#define DNET_ASSERT_HPP_

#include <cstdlib>
#include <iostream>
#include <string>

#define DNET_ASSERT(predicate, message)                                  \
  if (!(predicate)) {                                                    \
    std::cerr << "Assertion failed with message [" << message << "].\n"; \
    abort();                                                             \
  }

#endif  // DNET_ASSERT_HPP_
