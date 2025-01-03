// 进行参数率定
// Created by HenryChin on 2024/7/22.
//

//interface parameters
#include "../include/VIC_GA.h"
#include "map"
#include "iostream"
using namespace std;

// VIC 参数率定

// 全局使用一个参数
// 第一中情况是只考虑河流截面率定参数
// 第二种
// 找到不同参数
// 用每个flux的增散发加汇流的径流进行率定

void calibrate(map<string, string> &ga_config, map<string, string> &vic_config){


    convertASCIIToNetCDF(vic_config);  // 需要的话，把VIC3模式转为VIC4文件模式
//    generateForcingData(vic_config);
}
