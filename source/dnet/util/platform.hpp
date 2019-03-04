#ifndef PLATFORM_HPP_
#define PLATFORM_HPP_

// ============================================================ //
// Platform Detection
// ============================================================ //

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32) || \
    defined(__MINGW64)
#define DNET_PLATFORM_WINDOWS
#elif defined(__linux__)
#define DNET_PLATFORM_LINUX
#elif defined(__APPLE__)
#define DNET_PLATFORM_APPLE
#endif

#endif  // PLATFORM_HPP_
