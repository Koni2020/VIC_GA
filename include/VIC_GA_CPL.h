// Various convenience functions for Common Portability Library in VIC_GA.
// Created by HenryChin on 2024/8/8.
// Various convenience functions for VIC_GA.
#include "string"
#include "vector"
#ifndef VIC_GA_VIC_CPL_H
#define VIC_GA_VIC_CPL_H

using std::vector;
using std::string;

vector<int> generateSequence(int n, int startValue);
vector<string> splitString(const string& s, const string& delimiters) ; // 分割字符串
vector<float> remove_duplicates(vector<float> vec); // 删除重复值
vector<vector<float>> readFileAsFloat(const string &filePath);
vector<vector<string>> readDelimWhitespaceParallel(const string &fileName);
vector<string> getFilesNameInDirectory(const string &directory_path); // 获取文件夹名称
vector<string> getFilesInDirectory(const string &directory); // 获取文件夹路径
void getFileShape(const string &filePath, unsigned long &row_size, unsigned long & col_size);
void checkFileExist(const vector<string>& filesPath);
void checkRuntimeEnvironment(map<string, string> config);
template<typename T> // 找到索引值
unsigned long findFirstMatchIndex(const std::vector<T>& vec, const T& value) {
    auto it = find(vec.begin(), vec.end(), value);

    if (it != vec.end()) {
        return distance(vec.begin(), it);  // 返回找到的索引
    } else {
        return static_cast<unsigned long>(-1);  // 返回 -1 表示未找到
    }
}
#endif //VIC_GA_VIC_CPL_H
