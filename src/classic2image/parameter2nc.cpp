// convert ASCII file of VIC4 classic driver to netCDF file of VIC5 image driver
// Created by HenryChin on 2024/7/26.
//
#include "vector"
#include "string"
#include "list"
#include "netcdf"
#include "../../include/VIC_GA.h"
#include "../../include/VIC_GA_CPL.h"
#include "map"
#include "regex"
#include "boost/multi_array.hpp"
#include "fstream"
#include "cmath"

#define NLAYER 3


using namespace std;
using namespace netCDF;

using boost::multi_array;


void addDimVar(map<string, string> &vicConfig, map<string, NcDim> &paramDim,
               NcFile &file) {
/*
 * 给NCFIle加入LAT, LON, NBAND, VEGCLASS, 从而上面函数方便加入变量
 */
    vector<int> month = generateSequence(12, 1);
    const int root_zone = 3;
    const int nband = stoi(vicConfig["SNOW_BAND"]); // 雪带


    // 首先将土壤的第二列cell_num，第三列latitude，第四列longitude,加入纬度
    string path_soil_param = vicConfig["SOIL"];
    string path_veg_param = vicConfig["VEGPARAM"];
    unsigned long soilTableCol, soilTableRow;
    getFileShape(path_soil_param, soilTableRow, soilTableCol);
    auto soilParam = readFileAsFloat(path_soil_param);
    auto vegParam = readFileAsFloat(path_veg_param); // 太费时间

    vector<int> veg_class_container;
    int n = 0;
    while (n < vegParam.size()) {
        auto num = vegParam[n][1];
        num = static_cast<int> (num);
        veg_class_container.push_back(vegParam[n + num][0]);
        n += (num + 1);
    }
    auto veg_class = *max_element(veg_class_container.begin(), veg_class_container.end());
    veg_class += 1;

    vector<float> lat, lon;
    for (int i = 0; i < soilTableRow; ++i) {
        lat.push_back(soilParam[i][2]);
        lon.push_back(soilParam[i][3]);
    }

    vector<float> latitude = remove_duplicates(lat);
    vector<float> longitude = remove_duplicates(lon);

    reverse(latitude.begin(), latitude.end());


    const unsigned long lat_size = latitude.size();
    const unsigned long lon_size = longitude.size();

    NcDim layerDim = file.addDim("nlayer", NLAYER);
    NcDim latDim = file.addDim("lat", lat_size);
    NcDim lonDim = file.addDim("lon", lon_size);
    NcDim vegClassDim = file.addDim("veg_class", veg_class);
    NcDim rootZoneDim = file.addDim("root_zone", 3);
    NcDim snowBandDim = file.addDim("snow_band", nband);
    NcDim monthDim = file.addDim("month", 12);

    NcVar layerVar = file.addVar("nlayer", ncInt, layerDim);
    NcVar latVar = file.addVar("lat", ncFloat, latDim);
    NcVar lonVar = file.addVar("lon", ncFloat, lonDim);
    NcVar vegClassVar = file.addVar("veg_class", ncInt, vegClassDim);
    NcVar rootZoneVar = file.addVar("root_zone", ncInt, rootZoneDim);
    NcVar snowBandVar = file.addVar("snow_band", ncInt, snowBandDim);
    NcVar monthVar = file.addVar("month", ncInt, monthDim);

    layerVar.putVar(vector<int>{1, 2, 3}.data());
    latVar.putVar(latitude.data());
    lonVar.putVar(longitude.data());

    vegClassVar.putVar(generateSequence(veg_class, 1).data());
    rootZoneVar.putVar(generateSequence(root_zone, 1).data());
    snowBandVar.putVar(generateSequence(nband, 1).data());
    monthVar.putVar(generateSequence(12, 1).data());


    paramDim["layer"] = layerDim;
    paramDim["lat"] = latDim;
    paramDim["lon"] = lonDim;
    paramDim["veg"] = vegClassDim;
    paramDim["root"] = rootZoneDim;
    paramDim["snow"] = snowBandDim;
    paramDim["month"] = monthDim;

}


void getIndex(vector<float> &longitude, const vector<float> &latitude,
              float x, float y, unsigned long &loc_x, unsigned long &loc_y) { // 经度与纬度

    auto x_ = findFirstMatchIndex(longitude, x);
    auto y_ = findFirstMatchIndex(latitude, y);
    loc_x = x_;
    loc_y = y_;

}

void readSoilParams(const string &path_file,
                    const vector<bool> &is_multi_layers,
                    const vector<string> &column_name,
                    map<float, vector<unsigned long >> &cellNumIndex,
                    map<string, NcDim> &paramDim,
                    NcFile &file) {

    // 表的行数与列数
    unsigned long ncol;
    unsigned long nrow;


    getFileShape(path_file, nrow, ncol); // 获取表的形状
    vector<vector<float>> parameters;
    parameters = readFileAsFloat(path_file);

    size_t lat_size = paramDim["lat"].getSize();
    size_t lon_size = paramDim["lon"].getSize();

    unsigned long n = 0;
    for (int i = 0; i < is_multi_layers.size(); ++i) {
        if (i == 2 || i == 3) { // 这个是经纬度所以跳过
            n += 1;
            continue;
        }
        const string &var_name = column_name[i];
        if (is_multi_layers[i]) {
            if (var_name == "depth"){
                int a = 1;
            }
            float *param_var = new float[NLAYER * lat_size * lon_size];
            fill(param_var, param_var + NLAYER * lat_size * lon_size, NaN);
            for (int j = 0; j < nrow; ++j) {
                float cell_num = parameters[j][1];
                auto locX = cellNumIndex[cell_num][1];
                auto locY = cellNumIndex[cell_num][0];
                for (int k = 0; k < 3; ++k) {
                    int index = k * (lat_size * lon_size) + locY * lon_size + locX;
                    auto value = parameters[j][n + k];
                    param_var[index] = parameters[j][n + k];
                }
            }
            NcVar dataVar = file.addVar(var_name, ncFloat, {paramDim["layer"], paramDim["lat"], paramDim["lon"]});
            dataVar.putVar(param_var);
            delete[] param_var;
            n += 3;
        } else {
            if (var_name == "gridcell" | var_name == "run_cell") {
                int *param_var = new int[lat_size * lon_size];
                fill(param_var, param_var + lat_size * lon_size, 0);

                for (int j = 0; j < nrow; ++j)
                {
                    float cell_num = parameters[j][1];
                    auto locX = cellNumIndex[cell_num][1];
                    auto locY = cellNumIndex[cell_num][0];
                    int value = static_cast<int>(parameters[j][n]);
                    param_var[locY * lon_size + locX] = value;
                }
                NcVar dataVar = file.addVar(var_name, ncInt, {paramDim["lat"], paramDim["lon"]});
                dataVar.putVar(param_var);
                delete [] param_var;
                n += 1;
            } else {
                float *param_var = new float[lat_size * lon_size];
                for (int j = 0; j < nrow; ++j)
                {
                    float cell_num = parameters[j][1];
                    auto locX = cellNumIndex[cell_num][1];
                    auto locY = cellNumIndex[cell_num][0];
                    param_var[locY * lon_size + locX] = parameters[j][n];
                }
                NcVar dataVar = file.addVar(var_name, ncFloat, {paramDim["lat"], paramDim["lon"]});
                dataVar.putVar(param_var);
                delete [] param_var;
                n += 1;
            }

        }


    }
//
//    int b = 1;

}

void readVegParams(const string &path_file,
                   const vector<string> &veg_name,
                   map<float, vector<unsigned long >> &cellNumIndex,
                   map<string, NcDim> &paramDim,
                   NcFile &file) {


    auto param = readFileAsFloat(path_file);


    size_t class_size = paramDim["veg"].getSize();
    size_t lat_size = paramDim["lat"].getSize();
    size_t lon_size = paramDim["lon"].getSize();


    float *Cv = new float[class_size * lat_size * lon_size];
    float *depth = new float[class_size * lat_size * lon_size * 3];
    float *fraction = new float[class_size * lat_size * lon_size * 3];
    int *Nvg = new int[lat_size * lon_size];
    int *gridCel = new int[lat_size * lon_size];

    fill(Cv, Cv + class_size * lat_size * lon_size, 0);
    fill(depth, depth + class_size * lat_size * lon_size * 3, NaN);
    fill(fraction, fraction + class_size * lat_size * lon_size * 3, NaN);
    fill(gridCel, gridCel + lon_size * lat_size, -999);

    long long n = 0;
    while (n < param.size()) {
        auto cell_num = param[n][0];
        auto locX = cellNumIndex[cell_num][1];
        auto locY = cellNumIndex[cell_num][0];
        auto num = param[n][1];
        num = static_cast<int> (num);
        cell_num = static_cast<int> (cell_num);

        Nvg[locY * lon_size + locX] = num;
        gridCel[locY * lon_size + locX] = cell_num;

        for (int j = 1; j <= num; ++j) {
            int veg_class_ = param[n + j][0]; // 属于哪一类
            double cv = param[n + j][1];
            // class root lat lon
            vector<double> DBG;
            for (int k = 0; k < 3; ++k) {

                int index = veg_class_ * (lon_size * lat_size * 3) + k * (lat_size * lon_size) + locY * lon_size + locX;
                depth[index] = param[n + j][2 + k * 2]; // 2 4 6
                fraction[index] = param[n + j][3 + k * 2];
                DBG.push_back(param[n + j][2 + k * 2]);

            }

            int index = veg_class_ * (lon_size * lat_size) + locY * lon_size + locX;
            Cv[index] = cv;


        }

        n += (num + 1);
    }


    NcVar dataNveg = file.addVar("Nveg", ncInt, {paramDim["lat"], paramDim["lon"]});
    NcVar dataGridCel = file.addVar("grid_cel", ncInt, {paramDim["lat"], paramDim["lon"]});

    NcVar dataCv = file.addVar(veg_name[0], ncDouble, {paramDim["veg"], paramDim["lat"], paramDim["lon"]});
    NcVar dataDepth = file.addVar(veg_name[1], ncDouble,{paramDim["veg"], paramDim["root"], paramDim["lat"], paramDim["lon"]});
    NcVar dataFraction = file.addVar(veg_name[2], ncDouble,
                                     {paramDim["veg"], paramDim["root"], paramDim["lat"], paramDim["lon"]});

    dataCv.putVar(Cv);
    dataDepth.putVar(depth);
    dataFraction.putVar(fraction);
    dataNveg.putVar(Nvg);
    dataGridCel.putVar(gridCel);
    delete[] gridCel;
    delete[] Cv;
    delete[] depth;
    delete[] fraction;
    delete[] Nvg;
}

void readSnowParams(const string &path_file,
                    const int band,
                    const vector<string> &snow_name,
                    map<float, vector<unsigned long >> &cellNumIndex,
                    map<string, NcDim> &paramDim,
                    NcFile &file) {

    vector<vector<float>> parameters;
    unsigned long ncol;
    unsigned long nrow;
    size_t lat_size = paramDim["lat"].getSize();
    size_t lon_size = paramDim["lon"].getSize();

    getFileShape(path_file, nrow, ncol); // 获取表的形状

    parameters = readFileAsFloat(path_file);
    for (int i = 0; i < snow_name.size(); ++i) {
        float param_var[band][lat_size][lon_size];

        for (int ii = 0; ii < lat_size; ++ii) {
            for (int jj = 0; jj < lon_size; ++jj) {
                for (int kk = 0; kk < band; ++kk) {
                    param_var[kk][ii][jj] = NaN;
                }
            }
        } // 初始化

        for (int j = 0; j < nrow; ++j) {
            float cell_num = parameters[j][0];

            auto locX = cellNumIndex[cell_num][1];
            auto locY = cellNumIndex[cell_num][0];
            for (int k = 0; k < band; ++k) {
                auto var = parameters[j][1 + k + i * 6];
                param_var[k][locY][locX] = var;
            }
        }
        auto var_name = snow_name[i];
        NcVar dataVar = file.addVar(var_name, ncFloat, {paramDim["snow"], paramDim["lat"], paramDim["lon"],});
        dataVar.putVar(param_var);
    }
}

void getLocationXY(map<float, vector<unsigned long>> &cellNumIndex, const string &pathParameter) {
    // x 与 y 存放了经度与纬度的location, 基于cell
    unsigned long ncol;
    unsigned long nrow;

    getFileShape(pathParameter, nrow, ncol);

    vector<vector<float>> parameters = readFileAsFloat(pathParameter);
    // 循环转nc

    vector<float> lat, lon;

    for (int i = 0; i < nrow; ++i) {
        lat.push_back(parameters[i][2]);
        lon.push_back(parameters[i][3]);
    }

    vector<float> latitude = remove_duplicates(lat);
    vector<float> longitude = remove_duplicates(lon);

    reverse(latitude.begin(), latitude.end());
    for (int j = 0; j < nrow; ++j) {
        unsigned long locX, locY;
        auto id = parameters[j][1];
        getIndex(longitude, latitude, parameters[j][3], parameters[j][2], locX, locY);
        cellNumIndex[id] = vector<unsigned long>{locY, locX}; // 实际矩阵索引是行到列
    }

}


void readVegLib(const string &path_file, const vector<string> &vegLibName,
                map<float, vector<unsigned long >> &cellNumIndex,
                map<string, NcDim> &paramDim,
                NcFile &file) {

    size_t class_size = paramDim["veg"].getSize();
    size_t lat_size = paramDim["lat"].getSize();
    size_t lon_size = paramDim["lon"].getSize();
    size_t month_size = paramDim["month"].getSize();
    Table content = readDelimWhitespaceParallel(path_file);

    int n = 1;
    for (auto &i: vegLibName) {
        if ((i == "LAI") | (i == "albedo") | (i == "veg_rough") | (i == "displacement")) {
            float *tempArr = new float[class_size * month_size * lat_size * lon_size];
            fill(tempArr, tempArr + class_size * month_size * lat_size * lon_size, NaN);
            for (int mm = 0; mm < 12; ++mm) {
                for (int ii = 1; ii <= class_size; ++ii) { // 循环每一类
                    float vegLibValue = stof(content[ii][n]);
                    for (auto &jj: cellNumIndex) {
                        auto locX = jj.second[1];
                        auto locY = jj.second[0];
                        int index = (ii - 1) * (month_size * lon_size * lat_size) + mm * (lon_size * lat_size) +
                                    locY * lon_size + locX;
                        tempArr[index] = vegLibValue;
                    }
                }
                n += 1;
            }


            NcVar vegLibVar = file.addVar(i, ncFloat, {paramDim["veg"], paramDim["month"], paramDim["lat"],
                                                       paramDim["lon"]}); // 这里有问题！！！！记得回来改
            vegLibVar.putVar(tempArr);
            delete[] tempArr;
        } else {

            float *tempArr = new float[class_size * lat_size * lon_size];
            fill(tempArr, tempArr + class_size * lat_size * lon_size, NaN);
            for (int ii = 1; ii <= class_size; ++ii) { // 循环每一类
                float vegLibValue = stof(content[ii][n]);
                for (auto &jj: cellNumIndex) {
                    auto locX = jj.second[1];
                    auto locY = jj.second[0];
                    int index = (ii - 1) * (lon_size * lat_size) + locY * lon_size + locX;
                    tempArr[index] = vegLibValue;
                }
            }

            NcVar vegLibVar = file.addVar(i, ncFloat, {paramDim["veg"], paramDim["lat"], paramDim["lon"]});
            vegLibVar.putVar(tempArr);
            delete[] tempArr;
            n += 1;
        }
    }

}

void parameter2nc(map<string, string> &vic_config, string nc_path) {


    string soil_params_path = vic_config["SOIL"];
    string veg_params_path = vic_config["VEGPARAM"];
    string snow_params_path = vic_config["SNOW_PATH"];
    string veg_lib_path = vic_config["VEGLIB"];


    int snow_band = stoi(vic_config["SNOW_BAND"]);

    map<string, NcDim> paramDim; // initial netCDF dimension

    map<float, vector<unsigned long>> cellNumIndex; // index of cell num
    getLocationXY(cellNumIndex, soil_params_path);

    NcFile dataFile(nc_path, NcFile::replace); // initial file pointer to store data

    // step1 append dimension into netCDF file pointer
    addDimVar(vic_config, paramDim, dataFile);
    readSoilParams(soil_params_path, multi_layers,
                   soil_param_name, cellNumIndex, paramDim, dataFile);
    readVegLib(veg_lib_path, veg_lib_name, cellNumIndex, paramDim, dataFile);
    // step2 加入具体参数的值
    readVegParams(veg_params_path, veg_param_name, cellNumIndex, paramDim, dataFile);

    readSnowParams(snow_params_path, snow_band,
                   snow_param_name, cellNumIndex, paramDim, dataFile);

}
