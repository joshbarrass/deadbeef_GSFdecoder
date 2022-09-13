# Setup project
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(GSFdecoder)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_VERSION 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# add include dir
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/lib)

# add libraries
link_directories(${PROJECT_SOURCE_DIR}/lib)

# build viogsf
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/viogsf)

# build psflib
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/psflib)

# add sources and headers
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

# build library
add_library(GSFdecoder MODULE ${sources} ${headers})
set_target_properties(GSFdecoder PROPERTIES PREFIX "")
target_link_libraries(GSFdecoder viogsf psflib)
