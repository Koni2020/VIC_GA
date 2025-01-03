cmake_minimum_required(VERSION 3.8)
project(VIC_GA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(vic_ga src/classic2image/forcing2nc.cpp
        include/VIC_GA.h
        main.cpp
        src/calibrate.cpp
        src/run_vic.cpp
        src/rout_vic.cpp
        src/ga_algrithom.cpp
        src/read_config.cpp
        src/classic2image/parameter2nc.cpp
        src/vic_ga_cpl.cpp
        include/ThreadPool.h
        src/classic2image/domain2nc.cpp
        include/VIC_GA_CPL.h
        src/ascii2nc_warp.cpp
        src/met_sim/generate_forcing_data.cpp
)


find_library(NCXXPATH netcdf_c++4)
find_library(GDALPATH gdal)

find_package(Boost REQUIRED)

target_link_libraries(vic_ga ${NCXXPATH})
target_link_libraries(vic_ga ${Boost_LIBRARIES})
target_link_libraries(vic_ga ${GDALPATH})







