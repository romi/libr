#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include "test_file_utils.h"

std::string ReadFileAsString(const std::string& filePath)
{
    std::ostringstream contents;
    try
    {
        std::ifstream in;
        in.exceptions(std::ifstream::badbit | std::ifstream::failbit);
        in.open(filePath);
        contents << in.rdbuf();
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what();
        throw(ex);
    }

    return(contents.str());
}

void MakeFile(const std::string fileName, const std::string& buffer)
{
    remove(fileName.c_str());
    std::ofstream outFile(fileName, std::ios::trunc | std::ios::binary);
    if (outFile)
    {
        outFile.write(buffer.c_str(), static_cast<std::streamsize>(buffer.size()));
    }
    else
    {
        std::cout << "Failed to create file : " << fileName;
    }
}

bool ReadFileAsVector(std::string& filename, std::vector<uint8_t>& out)
{
        try{
                std::ifstream in(filename, std::ios::in | std::ios::binary);
                in.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                std::copy((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>(), std::back_inserter(out));
        }
        catch (const std::exception& ex)
        {
                std::cout << ex.what();
                throw(ex);
        }

        return true;
}

bool WriteVectorAsFile(std::string& filename, std::vector<uint8_t>& in)
{
        try{
                std::ofstream out(filename, std::ios::in | std::ios::binary);
                out.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                std::copy(in.begin(), in.end(), std::ostream_iterator<uint8_t>(out));
        }
        catch (const std::exception& ex)
        {
                std::cout << ex.what();
                throw(ex);
        }

        return true;
}