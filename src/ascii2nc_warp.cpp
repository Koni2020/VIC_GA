//
// Created by HenryChin on 2024/8/9.
//
#include "string"
#include "map"
#include "../include/VIC_GA.h"
#include "iostream"
#include "netcdf"
using namespace std;
using namespace netCDF;

void convertASCIIToNetCDF(map<string, string> vic_config){
    // initial dimension

    map<string, NcDim> paramDim;
//
    cout << "Read configuration finish\n";
    cout << "Detected VIC4 ASCII configuration files\n";
    parameter2nc(vic_config, "D:/VIC_GA/VIC5_RedRiver_parameters.nc");
    cout << "Convert VIC4 ASCII parameter file to netCDF file finish\n";
    domain2nc(vic_config, "D:/VIC_GA/VIC5_RedRiver_domain.nc");
    cout << "Convert VIC4 ASCII domain file to netCDF file finish\n";
    forcing2nc(vic_config, "D:/VIC_GA/VIC5_RedRiver_forcing.nc");
    cout << "Convert VIC4 ASCII forcing file to netCDF file finish\n";
}