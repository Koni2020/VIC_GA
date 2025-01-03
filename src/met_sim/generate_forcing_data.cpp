//
// Created by HenryChin on 2024/8/13.
//
#include "filesystem"
#include "iostream"
#include "map"
using namespace std;
using filesystem::path;
namespace fs = filesystem;
void generateForcingData(map<string, string> vic_config){
    path currentPath = fs::current_path();
    path newFolder = currentPath / "metsim forcing";
    if (fs::exists(newFolder)) {
        std::cout << "Directory already exists: " << newFolder << std::endl;
    } else {
        if (fs::create_directory(newFolder)) {
            std::cout << "Directory created: " << newFolder << std::endl;
        } else {
            std::cerr << "Failed to create directory: " << newFolder << std::endl;
        }
    }

    system("wsl -e bash -c \"source /root/anaconda3/bin/activate && conda activate base && ms && echo 'Conda environment activated'\"");
}