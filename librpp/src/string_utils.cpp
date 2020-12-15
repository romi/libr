#include "string_utils.h"

namespace rpp
{
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
        auto size = std::vsnprintf(nullptr, 0, format, ap);
        std::vector<char> output(++size, '\0');
        std::vsnprintf(&output[0], size, format, ap_copy);
        va_end(ap_copy);
        instring = &output[0];
    }

}


