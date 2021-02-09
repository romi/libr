/*
  ROMI libr

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  The libr library provides some hardware abstractions and low-level
  utility functions.

  libr is free software: you can redistribute it and/or modify it
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
#ifndef R_MEMBUFFER_H
#define R_MEMBUFFER_H
#include <mutex>
#include <vector>

namespace rpp
{
    class MemBuffer
    {
    public:
        MemBuffer() : data_(), mutex_()
        {}
        virtual ~MemBuffer() = default;
        void put(char c);
        void append(const char *data, int len);
        void append_zero();
        void append_string(const char *string);
        void printf(const char* format, ...);
        std::vector<char>& data();
        std::string string();
        void clear();
        size_t size();
        std::mutex& mutex();


    private:
        std::vector<char> data_;
        std::mutex mutex_;
    };
}

//// Use with caution!
//void membuf_set_len(membuf_t *b, int len);


#endif // _R_MEMBUF_H_
