#ifndef CPPUTIL_H
#define CPPUTIL_H

// Utility functions for C++ code

#include <string>

namespace cpputil {

// See: https://codereview.stackexchange.com/questions/187183/create-a-c-string-using-printf-style-formatting
std::string format(const char *fmt, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 1, 2)))
#endif
    ;

}

#endif // CPPUTIL_H
