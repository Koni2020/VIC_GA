// 使用遗传算法进行VIC模型参数率定，
// mode 1:
// Created by HenryChin on 2024/7/22.
//
#include "string"
#include "vector"
#include "map"
#include "variant"
#ifndef VIC_GA_VIC_GA_H
#define VIC_GA_VIC_GA_H

// https://vic.readthedocs.io/en/master/Documentation/Drivers/Image/Params/
// true is multi layers, and false is signal layers
using std::vector;
using std::string;
using std::map;
typedef vector<vector<string>> Table;


const vector<bool> multi_layers = {
        false, false, false, false, false,
        false, false, false, false,
        true, true, true, true,
        false,
        true,
        false, false,
        true, true, true, true, true, true, true,
        false,
        true, true,
        false, false, false,
        true, false
};
// the namelist of soil parameters
const vector<string> soil_param_name = {
        "run_cell",
        "gridcell",
        "lat",
        "lon",
        "infilt",
        "Ds",
        "Dsmax",
        "Ws",
        "c",
        "expt",
        "Ksat",
        "phi_s",
        "init_moist",
        "elev",
        "depth",
        "avg_T",
        "dp",
        "bubble",
        "quartz",
        "bulk_density",
        "soil_density",
        "organic",
        "bulk_dens_org",
        "soil_dens_org",
        "off_gmt",
        "Wcr_FRACT",
        "Wpwp_FRACT",
        "rough",
        "snow_rough",
        "annual_prec",
        "resid_moist",
        "fs_active"
}; // miss three parameters
const vector<string> veg_lib_name = {
        "overstory",
        "rarc",
        "rmin",
        "LAI",
        "albedo",
        "veg_rough",
        "displacement",
        "wind_h",
        "RGL",
        "rad_atten",
        "wind_atten",
        "trunk_ratio",
};
// 植被参数名称 除Vegetation Libarary parameters以外的
const vector<string> veg_param_name = {
        "Cv",
        "root_depth",
        "root_fract"
};

// The name list of snow parameters
const vector<string> snow_param_name = {
        "AreaFract",
        "elevation",
        "Pfactor"
};

#define NaN numeric_limits<float>::quiet_NaN()
void readConfig(const string& config_path, map<string, string > &config);
void runVIC(); // 进行初步的vic模拟
void calibrate(map<string, string> &ga_config, map<string, string> &vic_config); // 基于GA算法进行率定
void forcing2nc(map<string, string> &vic_config, string nc_path); // forcing 转为nc
void parameter2nc(map<string, string> &vic_config, string nc_pathg); // 参数转nc
void domain2nc(map<string, string> &vic_config, string nc_path); // 标准网格转netcdf
void convertASCIIToNetCDF(map<string, string> vic_config);
void generateForcingData(map<string, string> vic_config);
extern void rout_vic(); // 汇流
#endif //VIC_GA_VIC_GA_H
