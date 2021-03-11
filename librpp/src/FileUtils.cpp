#include "FileUtils.h"

void FileUtils::TryReadFileAsVector(const std::string &filename, std::vector <uint8_t> &out) {
        try {
                std::ifstream in(filename, std::ios::in | std::ios::binary);
                in.exceptions(std::ifstream::badbit | std::ifstream::failbit);
                std::copy((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>(),
                      std::back_inserter(out));
        }
        catch (const std::istream::failure &ex) {
                FILE_UTILS_EXCEPTION_LOG( "\"" << filename.c_str() << "\"" << " " << ex.what())
                throw;
        }
}

void FileUtils::TryWriteVectorAsFile(const std::string& filename, const std::vector<uint8_t>& in)
{
        try{
                std::ofstream out(filename, std::ios::out | std::ios::binary);
                out.exceptions(std::ofstream::badbit | std::ofstream::failbit);
                std::copy(in.begin(), in.end(), std::ostream_iterator<uint8_t>(out));
        }
        catch (const std::ostream::failure& ex) {
                FILE_UTILS_EXCEPTION_LOG( "\"" << filename.c_str() << "\"" << " " << ex.what())
                throw;
        }

}
