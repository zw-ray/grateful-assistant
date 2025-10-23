project(providers_extension)

set(target_name providers)

set(PROVIDER_SOURCES
    OllamaProvider.cpp
    OllamaProvider.h
    ProvidersExtension.cpp
    ProvidersExtension.h
    SiliconFlowModel.h
    SiliconFlowModelExecutors.cpp
    SiliconFlowModelExecutors.h
    SiliconFlowProvider.cpp
    SiliconFlowProvider.h
)

add_library(${target_name} SHARED ${PROVIDER_SOURCES})

set_target_properties(${target_name} 
    PROPERTIES 
        OUTPUT_NAME ${target_name}
        PREFIX ""
        SUFFIX ".ext"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_include_directories(${target_name} 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${target_name}
    PRIVATE
        kernel
        ai
        db
)


