project(GratefulAssistantExecutable)

set(target_name grateassistant)

add_executable(${target_name} main.cpp)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

target_include_directories(${target_name} PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${target_name} PRIVATE
    gui
    Qt6::Widgets
    db
    ai
)

set_target_properties(${target_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)