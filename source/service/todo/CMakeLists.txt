project(todo)

set(target_name todo)

set(TODO_HEADERS
    ${CMAKE_SOURCE_DIR}/include/service/todo/TodoService.h
)

set(TODO_SOURCES
    TodoAssistant.cpp
    TodoAssistant.h
    TodoIntent.h
    TodoService.cpp
)

add_library(${target_name} SHARED ${TODO_HEADERS} ${TODO_SOURCES})

target_include_directories(${target_name} 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${target_name} 
    PRIVATE 
        kernel
        db
        ai
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

# 拷贝 todo.prompt 到 bin 目录
add_custom_command(TARGET ${target_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/resource/todo.prompt
        ${CMAKE_BINARY_DIR}/bin/prompts/todo.prompt
)
