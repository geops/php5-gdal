#ifndef __debug_h__
#define __debug_h__

#include <cstdio>

// toggle this define to enable/disable the debug mode
// #define DEBUG 1

// these two macros convert macro values to strings
#define STRINGIFY2(x)   #x
#define STRINGIFY(x)    STRINGIFY2(x)


#if DEBUG

#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

#define DEBUG_LOG_MAXLEN 1000

#define DEBUG_LOG(MSG, ...) { \
            char debug_msg[DEBUG_LOG_MAXLEN]; \
            snprintf(&debug_msg[0], DEBUG_LOG_MAXLEN, __FILE__ ":" STRINGIFY(__LINE__) " " MSG, ##__VA_ARGS__); \
            php_log_err(debug_msg); \
    }

#define DEBUG_LOG_FUNCTION  DEBUG_LOG("%s", __func__)

#else // no debug

#define DEBUG_LOG(MSG, ...)
#define DEBUG_LOG_FUNCTION

#endif

#endif
