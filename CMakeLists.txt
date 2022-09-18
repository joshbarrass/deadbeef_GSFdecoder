set(CMAKE_INSTALL_PREFIX "/opt/deadbeef/lib/deadbeef")

# Setup project
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(GSFdecoder)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_VERSION 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# set build type variable in preprocessor
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _upper_build_type)
    add_compile_definitions(BUILD_${_upper_build_type}=1)
endif()

OPTION (ENABLE_LOGGING "Write trace messages to deadbeef's log" OFF)
IF(ENABLE_LOGGING)
  add_compile_definitions(ENABLE_LOGGING=1)
ENDIF()

OPTION (STDERR_DEBUGGING "Write trace messages to stderr" OFF)
IF(STDERR_DEBUGGING)
  add_compile_definitions(STDERR_DEBUGGING=1)
ENDIF()

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
set_property(TARGET GSFdecoder PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(GSFdecoder PROPERTIES PREFIX "")
target_link_libraries(GSFdecoder viogsf psflib)

install(TARGETS GSFdecoder LIBRARY DESTINATION .)
