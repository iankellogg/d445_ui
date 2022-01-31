cmake_minimum_required(VERSION 3.10)
project(lvgl)

set(CMAKE_PREFIX_PATH "/usr/local")
set(CMAKE_C_STANDARD 11)#C11
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(INCLUDE_DIRS "D445UI")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${INCLUDE_DIRS})

file(GLOB_RECURSE INCLUDES "lv_drivers/*.h"   "lvgl/*.h"  "./*.h" "D445UI/*.h" "D445UI/*/*.h" "lv_fs_if/*.h" "D445UI/*.hpp")
file(GLOB_RECURSE SOURCES  "lv_drivers/*.c"   "lvgl/*.c" "D445UI/*.c" "D445UI/*/*.c" "lv_fs_if/*.c" "D445UI/*.cpp")


# Proto file

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
SET(CMAKE_CXX_FLAGS "-O3")

find_package(SDL2 REQUIRED SDL2)
include_directories(${SDL2_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})


#-Lopencv_core -Lopencv_imgproc -Lopencv_imgcodecs -Lopencv_highgui -Lopencv_ml -Lopencv_videoio -Lopencv_video -Lopencv_features2d -Lopencv_calib3d -Lopencv_objdetect -Lopencv_flann

#find_package(opencv REQUIRED opencv_core opencv_imgproc opencv_imgcodecs opencv_ml opencv_videoio opencv_video opencv_features2d opencv_calib3d opencv_objdetect opencv_flann)
# find_library(opencv_location opencv  PATH_SUFFIXES opencv)
# message(STATUS ${opencv_location})
# find_path(OPENCV_HEADER_PATH core.hpp PATH_SUFFIXES opencv)
# target_include_directories(main PUBLIC ${OPENCV_HEADER_PATH})

# add_library(opencv SHARED IMPORTED)
# set_target_properties(opencv PROPERTIES IMPORTED_LOCATION ${pigpio_location})

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(main main.c  ${SOURCES} ${INCLUDES})
add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(main PRIVATE SDL2 m iw ${OpenCV_LIBS} )
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main)



