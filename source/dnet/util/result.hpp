#ifndef RESULT_HPP_
#define RESULT_HPP_

#include "dnet/util/types.hpp"

namespace dnet {

using ResultUnderlyingType = u8_fast;
enum class Result : ResultUnderlyingType
{
  kSuccess = 0,
  kFail
};

}

#endif//RESULT_HPP_
