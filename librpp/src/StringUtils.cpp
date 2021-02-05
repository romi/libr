// http://www.martinbroadhurst.com/how-to-trim-a-stdstring.html

#include <string>
#include "StringUtils.h"

namespace StringUtils
{
    std::string& ltrim(std::string& str, const std::string& chars)
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    std::string& rtrim(std::string& str, const std::string& chars)
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }

    std::string& trim(std::string& str, const std::string& chars)
    {
        return ltrim(rtrim(str, chars), chars);
    }

    void string_printf(std::string& instring, const char* format, ...)
    {
        va_list ap;
        va_start(ap, format);
        string_vprintf(instring, format, ap);
        va_end(ap);
    }

    void string_vprintf(std::string& instring, const char* format, va_list ap)
    {
        va_list ap_copy;
        va_copy(ap_copy, ap);
        size_t size = (size_t)std::vsnprintf(nullptr, 0, format, ap);
        std::vector<char> output(++size, '\0');
        std::vsnprintf(&output[0], size, format, ap_copy);
        va_end(ap_copy);
        instring = output.data();
    }
}
