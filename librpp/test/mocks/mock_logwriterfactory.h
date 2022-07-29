#include "ILogWriter.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
namespace rpp {
        class MockLogWriterFactory : public ILogWriterFactory {
        public:
            MOCK_METHOD(std::shared_ptr<ILogWriter>, create_console_writer, (), (override));
            MOCK_METHOD(std::shared_ptr<ILogWriter>, create_file_writer, (), (override));
        };
}
#pragma GCC diagnostic pop
