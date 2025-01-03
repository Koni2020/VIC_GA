#include <iostream>
#include "vector"
#include "algorithm"
#include "netcdf"  // used netcdf-cxx
#include "fstream"
#include "../../include/VIC_GA.h"
#include "boost/multi_array.hpp"
#include "thread"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <mutex>
#include "../../include/ThreadPool.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "../../include/VIC_GA_CPL.h"

/**
 * The forcing data must be chunked by calendar year, with each NetCDF file named by the year, e.g. prefix.$year.nc.
 * https://vic.readthedocs.io/en/master/Documentation/Drivers/Image/ForcingData/
*/

using namespace std;
using namespace netCDF;
using boost::extents;
using boost::multi_array;
using std::fill_n;
using namespace boost::gregorian;


vector<string> split(const string &str, char delimiter) { // this function is to split string bt delimiter
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


// 读取文件内容到字符串向量
void parseCoordinate(const string &filename, float *x, float *y) { // 提取文件名的坐标
    vector<string> xy = split(filename, '_');

    *x = stof(xy[1]);
    *y = stof(xy[2]);
}

void getObservationDateSeries(const string& file, vector<date> &observationDate) {

    auto content = readFileAsFloat(file);
    for (auto &i: content) {
        int y = i[0];
        int doy = i[1];
        date date_;
        date_ = date(y, 1, 1) + date_duration(doy);
        observationDate.push_back(date_);
    }
}


mutex mtx;

void processFile(int i, const vector<string> &filesName, const vector<string> &filesPath,
                 const vector<float> &lon, const vector<float> &lat
                 , vector<unsigned long> &locationObservationDate,
                 vector<float*> &all_data) {

    auto nlon = lon.size();
    auto nlat = lat.size();
    auto days = locationObservationDate.size();
    string file_name = filesName[i];
    string file_path = filesPath[i];

    float x, y;

    parseCoordinate(file_name, &y, &x);
    unsigned long loc_x = findFirstMatchIndex(lon, x);
    unsigned long loc_y = findFirstMatchIndex(lat, y);

    // 读取文件内容
    auto observationData = readFileAsFloat(file_path);

    // 将 simuData 的内容复制到 all_data[loc_y][loc_x] 中
    lock_guard<std::mutex> lock(mtx); // 确保 all_data 的线程安全
    for (int j = 0; j < days; ++j) {
        for (int k = 0; k < 4; ++k) {
            unsigned long loc_t = locationObservationDate[j];

            auto pointerArr = all_data[k];
            int index = j * (nlon * nlat) + loc_y * nlon + loc_x;
            float value = observationData[loc_t][k + 2];
            pointerArr[index] = value;
        }
    }
    cout << "Read\t" << i << "\t" << file_name << "\n";
}




void forcing2nc(map<string, string> &vic_config, string nc_path) {

    vector<date> observationDate;

    int startYear = stoi(vic_config["STARTYEAR"]);
    int startMonth = stoi(vic_config["STARTMONTH"]);
    int startDay = stoi(vic_config["STARTDAY"]);

    int endYear = stoi(vic_config["ENDYEAR"]);
    int endMonth = stoi(vic_config["ENDMONTH"]);
    int endDay = stoi(vic_config["ENDDAY"]);

    int timeStep = stoi(vic_config["TIME_STEP"]);



//    timeVar.putAtt("units", units);
//    timeVar.putAtt("calendar", "standard");

    date startDate(startYear, startMonth, startDay);
    date endDate(endYear, endMonth, endDay);

    date_duration days_diff = endDate - startDate;

    string startDateString = to_iso_extended_string(startDate);

    string timeUnits = "days since " + startDateString + " 00:00:00";

    const int days = days_diff.days(); // 率定天数
    string forcing_path = vic_config["FORCING1"];

    vector<int> timeVect = generateSequence(days, 0);

//    for (auto &i: timeVect){
//        i = i * timeStep;
//    }

    vector<string> filesName = getFilesNameInDirectory(forcing_path);
    vector<string> filesPath = getFilesInDirectory(forcing_path);
    getObservationDateSeries(filesPath[0], observationDate);

    vector<unsigned long> locationObservationDate;

    unsigned long loc;
    for (int i = 1; i <= days; ++i) {
        loc = findFirstMatchIndex(observationDate, startDate + date_duration(i));


        locationObservationDate.push_back(loc);
    }


    vector<float> longitude;
    vector<float> latitude;
    for (const string &file_name: filesName) {
        vector<string> xy = split(file_name, '_');
        float x, y;
        parseCoordinate(file_name, &x, &y);
        longitude.push_back(y);
        latitude.push_back(x);

    }
    vector<float> lon = remove_duplicates(longitude); // 去除重复值后的经纬度
    vector<float> lat = remove_duplicates(latitude);

    reverse(lat.begin(), lat.end());



    unsigned int lat_size;
    unsigned int lon_size;

    lat_size = lon.size();
    lon_size = lat.size();

    int nVar = 4;

    vector<float*> container;
    for (int i = 0; i < nVar; ++i) {
        auto* p = new float[lat_size * lon_size * days];
        fill(p, p + days * lat_size * lon_size, NaN);
        container.push_back(p);
    }



////     文件过多需要并行读入
    ThreadPool pool(15);

    vector<thread> threads;
    for (int i = 0; i < filesPath.size(); ++i) {
        threads.emplace_back(processFile, i, ref(filesName), ref(filesPath), ref(lon), ref(lat),
                             ref(locationObservationDate), ref(container));
    }

    for (auto &t: threads) {
        if (t.joinable()) {
            t.join();
        }
    }


    NcFile dataFile(nc_path, NcFile::replace);

    // 定义维度
    NcDim daysDim = dataFile.addDim("time", days);
    NcDim latDim = dataFile.addDim("lat", lat.size());
    NcDim lonDim = dataFile.addDim("lon", lon.size());

    // 定义变量
    NcVar latVar = dataFile.addVar("latitude", ncFloat, latDim);
    NcVar lonVar = dataFile.addVar("longitude", ncFloat, lonDim);
    NcVar timeVar = dataFile.addVar("time", ncFloat, daysDim);
    timeVar.putAtt("units", timeUnits);
    timeVar.putAtt("calendar", "proleptic_gregorian");


    timeVar.putVar(timeVect.data());
    // 写入坐标数据
    latVar.putVar(lat.data());
    lonVar.putVar(lon.data());

    //
    NcVar varTp = dataFile.addVar("precp", ncFloat, {daysDim, latDim, lonDim});
    NcVar varTmax = dataFile.addVar("tmax", ncFloat, {daysDim, latDim, lonDim});
    NcVar varTmin = dataFile.addVar("tmin", ncFloat, {daysDim, latDim, lonDim});
    NcVar varWd = dataFile.addVar("wind", ncFloat, {daysDim, latDim, lonDim});


    varTp.putVar(container[0]);
    varTmax.putVar(container[1]);
    varTmin.putVar(container[2]);
    varWd.putVar(container[3]);
    for (auto &i: container) {
        delete [] i;
    }
    cout << "NetCDF file created successfully.\n" << endl;

}



