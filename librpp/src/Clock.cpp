/*
  rcom

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  rcom is light-weight libary for inter-node communication.

  rcom is free software: you can redistribute it and/or modify it
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

#include <iomanip>
#include <chrono>
#include <thread>
#include "Clock.h"

namespace rpp {

        const int MILLISECONDS_IN_SECOND = 1000;
        double Clock::time()
        {
                using namespace std::chrono;
                milliseconds millisecs = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
                return static_cast<double>(millisecs.count()) / MILLISECONDS_IN_SECOND;
        }

        std::string Clock::datetime_compact_string()
        {
                std::time_t now =  std::time(nullptr);
                std::stringstream ss;
                ss << std::put_time(std::localtime(&now), "%Y%m%d-%H%M%S");
                return ss.str();
        }

    uint64_t Clock::timestamp() {
                auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
                auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
                return static_cast<uint64_t>(ns.count());
    }

    // TBD: Move this out of clock
    void Clock::sleep(double seconds) {
                long sleep_duration = long (seconds * MILLISECONDS_IN_SECOND);
                std::this_thread::sleep_for(std::chrono::milliseconds (sleep_duration));
        }
}

