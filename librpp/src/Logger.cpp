#include <iostream>
#include "Logger.h"

namespace rpp
{
    void Logger::MoveLog(std::filesystem::path newpath) {

        std::filesystem::path current;
        std::string filename{};

        if (r_log_get_file() != nullptr)
        {
            current = r_log_get_file();
            filename = current.filename();
        }
        else
            filename = "log.txt";

        newpath /= filename;

        try {
            r_log_cleanup();
            if (!current.empty())
                std::filesystem::rename(current, newpath);
            r_log_init();
            r_log_set_file(newpath.c_str());
        } catch (std::filesystem::filesystem_error& e) {
            std::cout << e.what() << '\n';
        }
    }
}
