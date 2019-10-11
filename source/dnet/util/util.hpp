/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <chif_net/chif_net.h>

namespace dnet {

inline void Startup() { chif_net_startup(); }

inline void Shutdown() { chif_net_shutdown(); }

/**
 * There is a soft and a hard limit to how many file descriptors you can
 * have open. This function allows you to change the soft limit.
 * @limit The new value, use NOFD_HARD_LIMIT to raise the soft limit to
 * be equal to hard limit.
 * @return If it was successful or not.
 */
bool SetNofdSoftLimit(int limit);

/**
 * @return the soft limit, or -1 if error.
 */
int GetNofdSoftLimit();

/**
 * @return the hard limit, or -1 if error.
 */
int GetNofdHardLimit();

constexpr int NOFD_HARD_LIMIT = -1;

}  // namespace dnet

#endif  // UTIL_HPP_
