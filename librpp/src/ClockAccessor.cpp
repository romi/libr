#include "ClockAccessor.h"

namespace rpp{
    std::shared_ptr< IClock > ClockAccessor::g_clock;
    std::mutex ClockAccessor::clock_mutex;


}