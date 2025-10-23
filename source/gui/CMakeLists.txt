# 声明项目名称
project(gui)

set(target_name gui)

# 查找Qt包 - 需要设置Qt6_DIR环境变量以指定Qt安装路径
find_package(Qt6 
    COMPONENTS 
        Core 
        Widgets
    REQUIRED
)

if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found.")
else()
    message(STATUS "Qt6 found.")
    message(STATUS "Qt6Widgets_INCLUDE_DIRS: ${Qt6Widgets_INCLUDE_DIRS}")
    message(STATUS "Qt6Core_INCLUDE_DIRS: ${Qt6Core_INCLUDE_DIRS}")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GUI_HEADERS
    ${CMAKE_SOURCE_DIR}/include/gui/MainWindow.h
)

# 添加源文件
set(GUI_SOURCES
    Card.cpp
    Card.h
    DateTimeCard.cpp
    DateTimeCard.h
    FloatMessage.cpp
    FloatMessage.h
    ListenContent.cpp
    ListenContent.h
    MainWindow.cpp
    TodoInput.cpp
    TodoInput.h
    TodoList.cpp
    TodoList.h
    TodoListItem.cpp
    TodoListItem.h
    WeatherCard.cpp
    WeatherCard.h
    ${CMAKE_SOURCE_DIR}/resource/res.qrc
)

include(${CMAKE_SOURCE_DIR}/cmake/options.cmake)

add_library(${target_name} ${TARGET_LIB_TYPE} ${GUI_SOURCES} ${GUI_HEADERS})

# 链接Qt库
target_link_libraries(${target_name} 
    PUBLIC 
        Qt6::Widgets
        Qt6::Core
        kernel
)

if(WIN32 )
    message(STATUS "Windows platform detected.")
    target_link_libraries(${target_name} 
        PRIVATE 
            ws2_32
            secur32
    )
endif()

string(TOUPPER ${target_name} TARGET_NAME_UPPER)
include(GenerateExportHeader)
generate_export_header(${target_name}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${target_name}/GuiExport.h
    EXPORT_MACRO_NAME ${TARGET_NAME_UPPER}_API
)

target_include_directories(${target_name} 
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE 
        ${Qt6Widgets_INCLUDE_DIRS}
        ${Qt6Core_INCLUDE_DIRS}
)

set_target_properties(${target_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)


if(WIN32)
    # 使用windeployqt工具，将Qt依赖项复制到可执行文件目录
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt REQUIRED HINTS ${Qt6_DIR}/../bin)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${target_name}>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMMENT "Running windeployqt on ${target_name}"
    )
endif()