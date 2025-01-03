//
// Created by HenryChin on 2024/7/26.
//
#include <sstream>
#include <vector>
#include <iostream>
#include "algorithm"
#include "fstream"
#include "regex"
#include "thread"
#include "../include/VIC_GA.h"
#include "numeric"
#include "filesystem"
#include "../include/Format.h"
#include <windows.h>
#include "array"
using namespace std;
namespace fs = std::filesystem;
const int BUFFER_SIZE = 1024;


vector<float> remove_duplicates(vector<float> vec) {
    // 先对vector进行排序
    sort(vec.begin(), vec.end());
    auto last = std::unique(vec.begin(), vec.end());
    vec.erase(last, vec.end());
    return vec;
}

vector<string> splitString(const string& s, const string& delimiters) {
    vector<std::string> tokens;
    regex re(delimiters);
    sregex_token_iterator it(s.begin(), s.end(), re, -1);
    sregex_token_iterator reg_end;
    for (; it != reg_end; ++it) {
        tokens.push_back(it->str());
    }
    return tokens;
}




void read_chunk(const string& file_name, streampos start, streamsize size, vector<string>& results, int chunk_id) {
    std::ifstream file(file_name, ios::in | ios::binary);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << file_name << endl;
        return;
    }

    file.seekg(start);
    vector<char> buffer(size);
    file.read(buffer.data(), size);

    results[chunk_id] = std::string(buffer.begin(), buffer.end());
}


Table readDelimWhitespaceParallel(const string &fileName) {
    // 改为多线程 2024/8/2
    const int num_threads = 1;

    ifstream file(fileName, ios::in | ios::binary | ios::ate);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << fileName << std::endl;
    }
    vector<char> buffer(BUFFER_SIZE);
    istringstream iss;
    streamsize file_size = file.tellg();
    file.close();

    vector<thread> threads; // 还需要加入线程池 进一步改进
    vector<string> results(num_threads);
    streamsize chunk_size = file_size / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        streampos start = i * chunk_size;
        streamsize size = (i == num_threads - 1) ? file_size - start : chunk_size;

        threads.emplace_back(read_chunk, fileName, start, size, ref(results), i);
    }

    for (auto &t: threads) {
        t.join();
    }

    std::string complete_file;
    for (const auto &chunk: results) {
        complete_file += chunk;
    }


    string delimiters = R"([\r\n]+)";
    auto contentSplitLine = splitString(complete_file, delimiters);

    Table contentLineDelimWhiteSpace;
    for (auto &i: contentSplitLine) {
        auto j = splitString(i, "\\s+");
        contentLineDelimWhiteSpace.push_back(j);
    }
    return contentLineDelimWhiteSpace;
}
vector<vector<float>> readFileAsFloat(const string &filePath){
    Table content = readDelimWhitespaceParallel(filePath);
    vector<vector<float>> results;
    for (auto &i: content) {
        vector<float> temp;
        for (auto &j: i) {
            if (j.empty()){
                continue;
            }
            temp.push_back(stof(j));
        }
        results.push_back(temp);
    }
    return results;
}


vector<int> generateSequence(int n, int startValue) {
    vector<int> sequence(n);
    iota(sequence.begin(), sequence.end(), startValue); // 填充序列从1开始
    return sequence;
}



vector<string> getFilesNameInDirectory(const string &directory_path) {
    vector<string> file_list;
    try {
        if (fs::exists(directory_path) && fs::is_directory(directory_path)) {
            for (const auto &entry: fs::directory_iterator(directory_path)) {
                if (fs::is_regular_file(entry.status())) {
                    file_list.push_back(entry.path().filename().string());
                }
            }
        } else {
            cerr << "Path does not exist or is not a directory: " << directory_path << std::endl;
        }
    } catch (const fs::filesystem_error &err) {
        cerr << "Filesystem error: " << err.what() << std::endl;
    }
    return file_list;
} // get filename

vector<string> getFilesInDirectory(const string &directory) {
    vector<string> files;

    for (const auto &entry: fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(fs::absolute(entry.path()).string());
        }
    }

    return files;
} // read absolute file path




void getFileShape(const string &filePath, unsigned long &row_size, unsigned long & col_size) { // 获取表形状

    unsigned long col_size_, row_size_;
    string line;

    ifstream infile(filePath);
    if (!infile.is_open()) {
        cerr << "Error: Could not open file " << filePath << endl;
    }

    // 读取文件内容
    row_size_ = 0;
    while (getline(infile, line)) {
        vector<string> line_ = splitString(line, "\t");
        if (row_size_ == 0) {
            col_size_ = line_.size();
        }

        row_size_ += 1;
    }
    row_size = row_size_;
    col_size = col_size_;
    infile.close();
}

void checkFileExist(const vector<string>& filesPath){

    for (auto &i: filesPath) {
        bool statues = fs::exists(i);
        if (!statues){
            cerr << util::Format("{} non exist, please check it\n", i);
            throw ;
        }
    }
}



void checkRuntimeEnvironment(map<string, string> ga_config){

    HKEY hKey;
    LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                              "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Lxss",
                              0,
                              KEY_READ,
                              &hKey);
    if (lRes == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        cout << "Detected WSL installed.\n";
    }else{
        throw runtime_error("WSL is not installed on this system.\n");
    }

    for (auto &i: vector<string> {"ms_path", "vic_path"}) {
        if (ga_config.find(i) == ga_config.end()) {
            // 如果键不存在，抛出异常
            throw runtime_error(util::Format("Variable {} does not exist in VIC GA configuration\n", i));
        }else{
            fs::path filePath = ga_config[i];
            if (!filesystem::exists(filePath)){
                throw runtime_error(util::Format("{} does not exist, please check it\n", i));
            } else{
                cout << util::Format("Detected {}: {}\n", filePath.filename().string(), filePath.string());
            }
        }
    }


}