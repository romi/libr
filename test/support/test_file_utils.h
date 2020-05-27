#ifndef ROMI_ROVER_BUILD_AND_TEST_TEST_FILE_UTILS_H
#define ROMI_ROVER_BUILD_AND_TEST_TEST_FILE_UTILS_H
#include <string>

std::string ReadFileAsString(const std::string& filePath);
void MakeFile(const std::string fileName, const std::string& buffer);

#endif //ROMI_ROVER_BUILD_AND_TEST_TEST_FILE_UTILS_H
