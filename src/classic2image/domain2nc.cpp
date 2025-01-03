//
// Created by HenryChin on 2024/8/6.
//
#include "iostream"
#include <map>
#include <string>
#include "gdal_priv.h"
#include "../../include/VIC_GA.h"
#include "algorithm"
#include "netcdf"
#include "../../include/VIC_GA_CPL.h"

using namespace netCDF;
using namespace std;
void readGeoArray(const char *tiffPath, float* arr, int band=1){
    // read the array of tiff at specific band into memory

    GDALAllRegister();
    GDALDataset *poDataset = (GDALDataset *)GDALOpen(tiffPath, GA_ReadOnly);
    GDALRasterBand *poBand = poDataset->GetRasterBand(band);
    int nXSize = poBand->GetXSize(); // length of longitude
    int nYSize = poBand->GetYSize(); // length of latitude
    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);
    float *pafScanline = (float *)CPLMalloc(sizeof(float) * nXSize);
    for (int i = 0; i < nYSize; i++)
    {
        poBand->RasterIO(GF_Read, 0, i, nXSize, 1,
                         pafScanline, nXSize, 1, GDT_Float32,
                         0, 0);

        for (int j = 0; j < nXSize; j++)
        {

            int index = i * nXSize + j;
            arr[index] = pafScanline[j];
        }
    }
}


void domain2nc(map<string, string> &vic_config, string nc_path){


    string pathGrid = "D:\\VIC_GA\\RUN_VIC\\VIC4_params_forcing\\domain\\YHST_grid.tif";
    string pathFraction = "D:\\VIC_GA\\RUN_VIC\\VIC4_params_forcing\\domain\\YHunion_fraction.tif";
    string pathElevation = "D:\\VIC_GA\\RUN_VIC\\VIC4_params_forcing\\domain\\YHdem_gridre01.tif";


    vector<float> lon, lat;
    float resolution;

    // 根据土壤参数来标准化坐标
    string path_soil_param = vic_config["SOIL"];
    unsigned long soilTableCol, soilTableRow;
    getFileShape(path_soil_param, soilTableRow, soilTableCol);
    auto soilParam = readFileAsFloat(path_soil_param);
    for (int i = 0; i < soilTableRow; ++i) {
        lat.push_back(soilParam[i][2]);
        lon.push_back(soilParam[i][3]);
    }

    vector<float> latitude = remove_duplicates(lat);
    vector<float> longitude = remove_duplicates(lon);

    reverse(latitude.begin(), latitude.end());
    size_t nXSize = longitude.size();
    size_t nYSize = latitude.size();


    float* gridStandard = new float [nXSize * nYSize];
    float* fraction = new float [nXSize * nYSize];
    float* cellArea = new float [nXSize * nYSize];
    float* elevation = new float [nXSize * nYSize];

    fill(fraction, fraction + nXSize * nYSize, NaN);
    fill(cellArea, cellArea + nXSize * nYSize, NaN);
    fill(elevation, elevation + nXSize * nYSize, NaN);

    // DBG
    readGeoArray(pathGrid.c_str(), gridStandard);
    readGeoArray(pathFraction.c_str(), fraction);
    readGeoArray(pathElevation.c_str(), elevation);

    for (int i = 0; i < nYSize; ++i) {
        for (int j = 0; j < nXSize; ++j) {
            int index = i * nXSize + j;
            cellArea[index] = resolution * resolution * 110 * 110 * fraction[index];
        }
    }
    NcFile file(nc_path, NcFile::replace);
    NcDim latDim = file.addDim("lat", nYSize);
    NcDim lonDim = file.addDim("lon", nXSize);

    NcVar latVar = file.addVar("lat", ncFloat, latDim);
    NcVar lonVar = file.addVar("lon", ncFloat, lonDim);
    NcVar gridVar = file.addVar("mask", ncInt, {latDim, lonDim});
    NcVar fractionVar = file.addVar("frac", ncFloat, {latDim, lonDim});
    NcVar cellAreaVar = file.addVar("area", ncFloat, {latDim, lonDim});
    NcVar elevationVar = file.addVar("elevation", ncFloat, {latDim, lonDim});

    latVar.putVar(latitude.data());
    lonVar.putVar(longitude.data());
    elevationVar.putVar(elevation);
    gridVar.putVar(gridStandard);
    fractionVar.putVar(fraction);
    cellAreaVar.putVar(cellArea);
    delete []gridStandard;
    delete []fraction;
    delete []cellArea;
    delete []elevation;
}