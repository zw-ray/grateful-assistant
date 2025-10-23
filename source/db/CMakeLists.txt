project(db)

set(target_name db)

set(DB_HEADERS
    ${CMAKE_SOURCE_DIR}/include/db/DatabaseConnection.h
    ${CMAKE_SOURCE_DIR}/include/db/DatabaseManager.h
)

set(DB_SOURCES
    DatabaseManager.cpp
    SiliconFlowModelInfoConnection.cpp
    SiliconFlowModelInfoConnection.h
    TodoConnection.cpp
    TodoConnection.h
)

add_library(${target_name} SHARED ${DB_HEADERS} ${DB_SOURCES})

find_package(unofficial-sqlite3 CONFIG REQUIRED)

target_link_libraries(${target_name} 
    PRIVATE
        unofficial::sqlite3::sqlite3
    PUBLIC
        kernel
)

string(TOUPPER ${target_name} TARGET_NAME_UPPER)
include(GenerateExportHeader)
generate_export_header(${target_name}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${target_name}/DatabaseExport.h
    EXPORT_MACRO_NAME DATABASE_API
)

target_include_directories(${target_name}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC 
        ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(${target_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)