cmake_minimum_required(VERSION 3.30)

project(GratefulAssistant)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 定义编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(GA_DEBUG)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(source/app)
add_subdirectory(source/gui)
add_subdirectory(source/db)
add_subdirectory(source/kernel)
add_subdirectory(source/ai)
add_subdirectory(source/service)

include(cmake/options.cmake)
# EXTENSIONS 
if(BUILD_AI_PROVIDERS)
    add_subdirectory(source/extensions/ai/providers)
endif()

if(BUILD_WEATHER_PROVIDERS)
    add_subdirectory(source/extensions/weather/providers)
endif()