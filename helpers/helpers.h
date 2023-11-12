#pragma once

#include <iostream>
#include <sstream>
#include <cassert>

#if (defined ARCH_WINDOWS)
    #include <Windows.h>
    #undef OPAQUE
#elif (defined ARCH_UNIX)
	#include <cstring>
	#include <errno.h>
#else
    #error "Unsupported platfrom, only Windows and UNIX like systems supported by helpers"
#endif

#if (! defined STR)
#define STR(...) static_cast<std::stringstream &&>(std::stringstream() << __VA_ARGS__).str()
#endif

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

#define ASSERT assert

#define NOT_IMPLEMENTED ASSERT(false && "Feature not implemented")

/** OS Specific error. 
 
    Automatically augments the message with OS specific information, such as error code or error string. 
 */
class OSError: public std::runtime_error {
public:
    OSError(std::string const & what):
        std::runtime_error{what} {
    }

    static OSError Patch(std::string const & what) {
#if (defined ARCH_WINDOWS)
        return OSError{STR(what << "(" << GetLastError() << ")")};
#elif (defined ARCH_UNIX) 
        return OSError{STR(what << " (" << strerror(errno) << ")")};
#else 
        return OSError{what};
#endif
    }
}; // OSError

/** Convenience macro for checking calls which return error using the platform specified way. 

    Executes its arguments and if they evaluate to false, throw OSError. The message of the OS error can be provided after the OSCHECK using the `<<` notion. 
 */
#define OSCHECK(...) if (! (__VA_ARGS__)) throw OSError::Patch("OS Error:")



inline bool isDecimalDigit(char c) { return c >= '0' && c <= '9'; }

inline bool isPrintableCharacter(char c) { return c >= 32 && c < 127; }

inline char nibbleToHex(uint8_t x) {
    if (x < 10)
        return '0' + x;
    ASSERT(x < 16);
    return 'a' - 10 + x;
}

inline uint8_t hexToNibble(char c) {
    if (c >= '0' && c <= '9')
        return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f')
        return static_cast<uint8_t>((c - 'a') + 10);
    if (c >= 'A' && c <= 'F')
        return static_cast<uint8_t>((c - 'A') + 10);
    throw std::invalid_argument{STR("Invalid hexadecimal character " << c)};    
}
