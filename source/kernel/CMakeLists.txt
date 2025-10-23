
set(target_name kernel)

project(${target_name})

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

set(KERNEL_HEADERS
    ${CMAKE_SOURCE_DIR}/include/kernel/Configuration.h
    ${CMAKE_SOURCE_DIR}/include/kernel/DynamicLinker.h
    ${CMAKE_SOURCE_DIR}/include/kernel/EventBus.h
    ${CMAKE_SOURCE_DIR}/include/kernel/Events.h
    ${CMAKE_SOURCE_DIR}/include/kernel/Extension.h
    ${CMAKE_SOURCE_DIR}/include/kernel/IService.h
    ${CMAKE_SOURCE_DIR}/include/kernel/Logger.h
    ${CMAKE_SOURCE_DIR}/include/kernel/ServiceManager.h
    ${CMAKE_SOURCE_DIR}/include/kernel/Syncronizer.h
    ${CMAKE_SOURCE_DIR}/include/kernel/WeatherInfo.h
)

set(KERNEL_SRC
    Configuration.cpp
    DynamicLinker.cpp
    EventBus.cpp
    Extension.cpp
    Logger.cpp
    ServiceManager.cpp
    IService.cpp
    Syncronizer.cpp
)

include(${CMAKE_SOURCE_DIR}/cmake/options.cmake)

add_library(${target_name} ${TARGET_LIB_TYPE} ${KERNEL_SRC} ${KERNEL_HEADERS})

string(TOUPPER ${target_name} TARGET_NAME_UPPER)
include(GenerateExportHeader)
generate_export_header(${target_name}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${target_name}/KernelExport.h
    EXPORT_MACRO_NAME ${TARGET_NAME_UPPER}_API
)

target_include_directories(${target_name}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC 
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${target_name} 
    PUBLIC 
        spdlog::spdlog_header_only
        nlohmann_json::nlohmann_json
        fmt::fmt-header-only
)

set_target_properties(${target_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# 拷贝配置文件
add_custom_command(TARGET ${target_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/resource/templates/appconfig.json
        $<TARGET_FILE_DIR:${target_name}>/config/appconfig.json
)
