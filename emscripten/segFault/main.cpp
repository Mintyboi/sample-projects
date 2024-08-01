#include <iostream>
#include <boost/filesystem/path.hpp>

#include<emscripten.h>

int main() {
    static 
    const auto& tempPath(boost::filesystem::path(L"/temp") / L"dir");

    std::wcout<< tempPath.wstring().c_str() << std::endl;
    return 0;
}