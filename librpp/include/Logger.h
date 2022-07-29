#ifndef ROMI_ROVER_BUILD_AND_TEST_LOGGER_H
#define ROMI_ROVER_BUILD_AND_TEST_LOGGER_H

#include <mutex>
#include <fstream>
#include <map>
#include "ILogWriter.h"
#include "ILogger.h"
#include "StringUtils.h"

namespace rpp
{

    class Logger : public ILogger
    {
    private:
        static std::string filename_;
        std::string application_name_;
        static std::mutex log_mutex_;
        static std::shared_ptr<ILogger> logger_;
        std::map<log_level, std::string> log_level_names_;
        const std::shared_ptr<ILogWriterFactory> logWriterFactory_;
        std::shared_ptr<ILogWriter> logWriter_;

    private:
        explicit Logger(const std::shared_ptr<ILogWriterFactory>& logWriterFactory);

    public:
        ~Logger() override = default;
        Logger(Logger &other) = delete;
        void operator=(const Logger &) = delete;
        static std::shared_ptr<ILogger> Instance();
        static void MoveLog(std::filesystem::path newpath);
        void log(log_level level, const char* format, ...) override;
        std::string get_log_file_path() override;
        void set_application_name(std::string_view application_name) override;

        void log_to_file(const std::string &log_path) override;
        void log_to_console() override;
    };
}
// Outside of namespace for now as we are replacing c functions.
// This enables us to not change the whole code base, whilst using a more testable C++ class under the hood.
int log_init();
void log_cleanup();
void log_set_application(std::string_view application_name);

int log_set_file(const std::string &path);
std::string log_get_file();

void log_set_console();

template <typename ...Args>
void r_err(const std::string format, Args && ...args) {
    rpp::Logger::Instance()->log(rpp::log_level::ERROR, format.c_str(), std::forward<Args>(args)...);
}

template <typename ...Args>
void r_warn(const std::string format, Args && ...args){
    rpp::Logger::Instance()->log(rpp::log_level::WARNING, format.c_str(), std::forward<Args>(args)...);
}

template <typename ...Args>
void r_info(const std::string format, Args && ...args){
    rpp::Logger::Instance()->log(rpp::log_level::INFO, format.c_str(), std::forward<Args>(args)...);
}

template <typename ...Args>
void r_debug(const std::string format, Args && ...args) {
    rpp::Logger::Instance()->log(rpp::log_level::DEBUG, format.c_str(), std::forward<Args>(args)...);
}

#endif
