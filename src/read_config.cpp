// 读取VIC_GA 以及 VIC 全局配置文件，并进行parsing
// Created by HenryChin on 2024/7/26.
//

#include "string"
#include "algorithm"
#include "vector"
#include "../include/VIC_GA.h"
#include "map"
#include "../include/VIC_GA_CPL.h"

using namespace std;



// 读取VIC_GA配置文件
void readConfig(const string& config_path, map<string, string> &config){

    Table config_param = readDelimWhitespaceParallel(config_path);
    for (auto &i:config_param) {
        if ((i[0] == "#") | (i.size() == 1)){  // 这一行是注释跳过
            continue;
        } else{
            config[i[0]] = i[1];
            if  (i[0] == "SNOW_BAND"){
                string snow_path = i[2]; // 比较特殊的一行，单独拿出来
                config["SNOW_PATH"] = snow_path;
            }
        }
    }// 读取文件内容

}
