#ifndef ROMI_ROVER_BUILD_AND_TEST_ILOGGER_H
#define ROMI_ROVER_BUILD_AND_TEST_ILOGGER_H

#include <filesystem>
#include "log.h"

namespace rpp
{
    class ILogger
    {
    public:
        ILogger() = default;
        virtual ~ILogger() = default;
    };
}
#endif //ROMI_ROVER_BUILD_AND_TEST_ILOGGER_H
