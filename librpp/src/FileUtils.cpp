#include "FileUtils.h"

bool FileUtils::ReadFileAsVector(const std::string &filename, std::vector <uint8_t> &out) {
        try {
                std::ifstream in(filename, std::ios::in | std::ios::binary);
                in.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                std::copy((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>(),
                      std::back_inserter(out));
        }
        catch (const std::exception &ex) {
                std::cout << ex.what();
                throw (ex);
        }

    return true;
}

bool FileUtils::WriteVectorAsFile(const std::string& filename, const std::vector<uint8_t>& in)
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
