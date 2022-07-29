#include "ILogWriter.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
namespace rpp {
        class MockLogWriter : public ILogWriter {
        public:
            MOCK_METHOD(void, open, (std::string_view name), (override));
            MOCK_METHOD(void, close, (), (override));
            MOCK_METHOD(void, write, (const std::string& message), (override));
        };
}
#pragma GCC diagnostic pop
