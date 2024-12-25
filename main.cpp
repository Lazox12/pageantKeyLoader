#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr<<"you need to enter at leat 2 args 1. path to pageant 2.at least 1 folder"<<std::endl;
        return 1;
    }
    std::string args;
    const char * process = argv[1];
    for (int i = 2; i < argc; i++) {
        for (const auto& file:fs::directory_iterator(argv[i])) {
            if (file.is_regular_file() and file.path().extension() ==".ppk") {
                args += " "+file.path().string();
            }
        }
    }
    std::cout<<process+args<<std::endl;
}