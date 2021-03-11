#ifndef ROMI_FILEUTILS_H
#define ROMI_FILEUTILS_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>

class FileUtils
{
public:
    static bool ReadFileAsVector(const std::string &filename, std::vector <uint8_t> &out);
    static bool WriteVectorAsFile(const std::string& filename, const std::vector<uint8_t>& in);
};


#endif //ROMI_ROVER_BUILD_AND_TEST_FILEUTILS_H
