// 基于GA算法对VIC进行参数率定，基于VIC5 image_drive
// Created by HenryChin on 2024/7/22.
//
#include "include/VIC_GA.h"
#include "include/VIC_GA_CPL.h"
#include <iostream>
#include "filesystem"
using namespace std;
using filesystem::path;

int main(){

    path currentPath = std::filesystem::current_path();
    cout << currentPath.string() <<'\n';
    cout << "######### Calibrate VIC Parameters based on GA Algorithm #########\n";
    string path_vic_ga_config = "D:\\VIC_GA\\vic_ga.config";
    if (!filesystem::exists(path_vic_ga_config)){
        throw runtime_error("vic ga configuration file does not exist");
    };
    // 读取配置文件
    map<string, string> ga_config; // ga 全局配置文件
    map<string, string> vic_config; // vic的全局配置文件

    readConfig(path_vic_ga_config, ga_config);
    readConfig(ga_config["global_param"], vic_config);
    checkRuntimeEnvironment(ga_config);

    calibrate(ga_config, vic_config);
    // DBUG
    return 0;
}