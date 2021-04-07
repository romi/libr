#ifndef ROMI_ROVER_BUILD_AND_TEST_CLOCKACCESSOR_H
#define ROMI_ROVER_BUILD_AND_TEST_CLOCKACCESSOR_H

#include <memory>
#include "Clock.h"

namespace rpp {

    class ClockAccessor{
            public:
                static const std::shared_ptr< IClock >& GetInstance()
                {
                        if (g_clock == nullptr)
                                g_clock = std::make_shared<rpp::Clock>();
                        return g_clock;
                }

                static void SetInstance(const std::shared_ptr< IClock >& globalClock)
                {
                      g_clock = globalClock;
                }

            private:
                static std::shared_ptr <IClock> g_clock;
    };
}

#endif //ROMI_ROVER_BUILD_AND_TEST_CLOCKACCESSOR_H
