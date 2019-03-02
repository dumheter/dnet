#ifndef RESULT_HPP_
#define RESULT_HPP_

#include "dnet/util/types.hpp"

namespace dnet {

using ResultUnderlyingType = bool;
enum class Result : ResultUnderlyingType
{
  kSuccess = true,
  kFail = false
};

}

#endif//RESULT_HPP_
