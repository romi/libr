// http://www.martinbroadhurst.com/how-to-trim-a-stdstring.html


#include <string>
#include <vector>
#include <stdarg.h>

namespace StringUtils
{
    const std::string whitespace_chars = "\t\n\v\f\r ";

    std::string& ltrim(std::string& str, const std::string& chars);
    std::string& rtrim(std::string& str, const std::string& chars);
    std::string& trim(std::string& str, const std::string& chars);
    void string_printf(std::string& instring, const char* format, ...);
    void string_vprintf(std::string& instring, const char* format, va_list ap);

    template <typename ...Args>
    std::string string_format(const std::string& format, Args && ...args)
    {
        size_t size = (size_t)std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...);
        std::vector<char> output(++size, '\0');
        std::snprintf(&output[0], size, format.c_str(), std::forward<Args>(args)...);
        return std::string(output.data());
    }
}


