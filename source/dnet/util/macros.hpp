#ifndef DNET_MACROS_HPP_
#define DNET_MACROS_HPP_

// ============================================================ //
// Macro Declaration
// ============================================================ //

// make this: "/path/to/file.c" into this: "file.c"
#if defined(DNET_PLATFORM_WINDOWS)
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

// ============================================================ //

// does it like this: [file.cpp:linenr]
#define CODE_LINK std::string("[") + std::string(__FILENAME__) + \
                  std::string(":") + std::to_string(__LINE__) + std::string("]")

#define THROW_CODE_LINK std::string(" [throw]@") + CODE_LINK

#define DNET_C_NOT_IMPLEMENTED static_assert(false, + CODE_LINK + \
                                    std::string("Not yet implemented."))

// ============================================================ //

#endif //DNET_MACROS_HPP_
