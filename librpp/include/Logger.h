#ifndef ROMI_ROVER_BUILD_AND_TEST_LOGGER_H
#define ROMI_ROVER_BUILD_AND_TEST_LOGGER_H
#include "ILogger.h"

namespace rpp
{
    class Logger : public ILogger
    {
    public:
        Logger() = default;
        ~Logger() override = default;
        static void MoveLog(std::filesystem::path newpath);
    };
}

#endif //ROMI_ROVER_BUILD_AND_TEST_LOGGER_H
