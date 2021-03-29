#include "IClock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
namespace rpp {
        class MockClock : public IClock {
        public:
                MOCK_METHOD0(time, double());
                MOCK_METHOD0(time_compact_string, std::string());
        };
}
#pragma GCC diagnostic pop
