#ifndef ROMI_ROVER_BUILD_AND_TEST_STRING_UTILS_H
#define ROMI_ROVER_BUILD_AND_TEST_STRING_UTILS_H

#include <stdarg.h>
#include <string>
#include <vector>

namespace rpp
{
    void string_printf(std::string& instring, const char* format, ...);
    void string_vprintf(std::string& instring, const char* format, va_list ap);

    template <typename ...Args>
    std::string string_format(const std::string& format, Args && ...args)
    {
        auto size = std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...);
        std::vector<char> output(++size, '\0');
        std::snprintf(&output[0], size, format.c_str(), std::forward<Args>(args)...);
        return std::string(&output[0]);
    }
}

#endif //ROMI_ROVER_BUILD_AND_TEST_STRING_UTILS_H
