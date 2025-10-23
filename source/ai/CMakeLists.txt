project(ai)

set(target_name ai)

set(AI_HEADERS
    ${CMAKE_SOURCE_DIR}/include/ai/AI.h
    ${CMAKE_SOURCE_DIR}/include/ai/AssistantRole.h
    ${CMAKE_SOURCE_DIR}/include/ai/Intent.h
    ${CMAKE_SOURCE_DIR}/include/ai/IntentManager.h
    ${CMAKE_SOURCE_DIR}/include/ai/Model.h
    ${CMAKE_SOURCE_DIR}/include/ai/Provider.h
    ${CMAKE_SOURCE_DIR}/include/ai/ProviderManager.h
    ${CMAKE_SOURCE_DIR}/include/ai/RoleManager.h
)

set(AI_SOURCES
    roles/AwakeWordVerifyRole.cpp
    roles/AwakeWordVerifyRole.h
    roles/SystemRole.cpp
    roles/SystemRole.h
    AI.cpp
    IntentManager.cpp
    Model.cpp
    Provider.cpp
    ProviderManager.cpp
    RoleManager.cpp
)

add_library(${target_name} SHARED ${AI_SOURCES} ${AI_HEADERS})

string(TOUPPER ${target_name} TARGET_NAME_UPPER)
include(GenerateExportHeader)
generate_export_header(${target_name}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${target_name}/AIExport.h
    EXPORT_MACRO_NAME ${TARGET_NAME_UPPER}_API
)

target_include_directories(${target_name} 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC 
        ${CMAKE_CURRENT_BINARY_DIR}
)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)

set(link_libs 
    nlohmann_json::nlohmann_json
    fmt::fmt
    httplib::httplib
    date::date 
    date::date-tz
    kernel
    db
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
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# 构建完成后将 ${CMAKE_SOURCE_DIR}/resource/templates/prompts 目录下的所有文件复制到 ${CMAKE_BINARY_DIR}/bin/prompts
# 并且每次必须执行
add_custom_command(
    TARGET ${target_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resource/templates/prompts
        ${CMAKE_BINARY_DIR}/bin/prompts
)