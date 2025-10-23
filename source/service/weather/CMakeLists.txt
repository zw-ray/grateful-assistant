project(weather)

set(target_name weather)

set(WEATHER_HEADERS
    ${CMAKE_SOURCE_DIR}/include/service/weather/WeatherService.h
)

set(WEATHER_SOURCES
    AMapInfo.h
    AMapIPLocator.cpp
    AMapIPLocator.h
    AMapWeatherFetcher.cpp
    AMapWeatherFetcher.h
    DDNSOrayFetcher.hpp
    IP3322Fetcher.hpp
    IPFetcher.cpp
    IPFetcher.h
    IPLocator.h
    MyIpFetcher.hpp
    WeatherFetcher.h
    WeatherService.cpp
    YinghualuoFetcher.hpp
)

add_library(${target_name} SHARED ${WEATHER_HEADERS} ${WEATHER_SOURCES})

find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_include_directories(${target_name}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

set(link_libs
    nlohmann_json::nlohmann_json
    httplib::httplib
    fmt::fmt
    kernel
)

# 如果是 mac 平台，需要链接 CoreFundation
if (APPLE)
    target_link_libraries(${target_name} PUBLIC "-framework CoreFoundation")
    target_link_libraries(${target_name} PUBLIC "-framework Security")
    target_link_libraries(${target_name} PUBLIC "-framework CFNetwork")
endif()

target_link_libraries(${target_name}
    PRIVATE 
        ${link_libs}
)

set_target_properties(${target_name} 
    PROPERTIES 
        OUTPUT_NAME ${target_name}
        PREFIX ""
        SUFFIX ".svc"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)