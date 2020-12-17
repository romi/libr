/*
  ROMI librpp

  Copyright (C) 20120 Sony Computer Science Laboratories
  Author(s) Peter Hanappe, Douglas Boari

  The librpp library provides some hardware abstractions and low-level
  utility functions.

  librpp is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include <cstring>

#include "log.h"
#include "MemBuffer.h"
#include "StringUtils.h"

namespace rpp
{
    void MemBuffer::put(char c)
    {
        data_.emplace_back(c);
    }

    void MemBuffer::append(const char *data, int len)
    {
        data_.insert(data_.end(), data, data+len);
    }

    void MemBuffer::append_zero()
    {
        data_.emplace_back(0);
    }

    void MemBuffer::append_string(const char *string)
    {
        const int KB_32 = (32 * 1024);
        size_t lens = strnlen(string, KB_32);
        if (lens == KB_32)
            r_warn("MemBuffer::append_str() string truncated to 32kb");
        data_.insert(data_.end(), string, string+lens);
    }

    void MemBuffer::printf(const char *format, ...)
    {
        va_list ap;
        va_start(ap, format);
        std::string formatted_string;
        StringUtils::string_vprintf(formatted_string, format, ap);
        va_end(ap);
        data_.insert(data_.end(), formatted_string.data(), formatted_string.data()+formatted_string.length());
        // might need to append 0 depending on use case.
        // append_zero();
    }

    std::vector<char>&
    MemBuffer::data()
    {
        return data_;
    }

    void MemBuffer::clear()
    {
        data_.clear();
    }

    size_t MemBuffer::size()
    {
        return data_.size();
    }

    std::mutex& MemBuffer::mutex()
    {
        return mutex_;
    }
}
