/**
 * Thanks Filip for most things in this header.
 */

#pragma once

// ============================================================ //
// Platform Detection
// ============================================================ //

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW64)
#define DNET_PLATFORM_WINDOWS
#elif defined(__linux__)
#define DNET_PLATFORM_LINUX
#elif defined (__APPLE__)
#define DNET_PLATFORM_APPLE
#endif

// ============================================================ //
// Platform Specific Headers
// ============================================================ //

#if defined(DNET_PLATFORM_WINDOWS)
#elif defined(DNET_PLATFORM_LINUX)
#elif defined(DNET_PLATFORM_APPLE)
#endif
