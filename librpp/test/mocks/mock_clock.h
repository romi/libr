#include "IClock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
namespace rpp {
        class MockClock : public IClock {
        public:
                MOCK_METHOD0(time, double());
                MOCK_METHOD0(datetime_compact_string, std::string());
                MOCK_METHOD0(timestamp, uint64_t ());
                MOCK_METHOD1(sleep, void (double));
        };
}
#pragma GCC diagnostic pop
