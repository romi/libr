#ifndef ROMI_FILEUTILS_H
#define ROMI_FILEUTILS_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <cstring>
#include <filesystem>

#include "ILinux.h"
namespace fs = std::filesystem;

#define FILE_UTILS_EXCEPTION_LOG(...) \
        std::cout << fs::path(__FILE__).filename() << ": " << __func__ << "(): " << __VA_ARGS__ << ": " << std::endl;

class FileUtils
{
public:
    static void TryReadFileAsVector(const std::string &filename, std::vector <uint8_t> &out);
    static void TryWriteVectorAsFile(const std::string& filename, const std::vector<uint8_t>& in);
    static fs::path TryGetHomeDirectory(rpp::ILinux& linux);
};


#endif //ROMI_ROVER_BUILD_AND_TEST_FILEUTILS_H
