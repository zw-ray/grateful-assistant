project(audio)
set(target_name audio)

add_subdirectory(webrtc_vad)

set(AUDIO_HEADERS
    ${CMAKE_SOURCE_DIR}/include/service/audio/AudioService.h
)

set(AUDIO_SOURCES
    AudioService.cpp
    AudioState.h
    AudioWorker.cpp
    AudioWorker.h
    AwaitContentState.cpp
    AwaitContentState.h
    AwaitWakewordState.cpp
    AwaitWakewordState.h
    PortaudioWrapper.cpp
    PortaudioWrapper.h
    VADDetector.cpp
    VADDetector.h
    VoicePrintDetector.cpp
    VoicePrintDetector.h
)

add_library(${target_name} SHARED ${AUDIO_HEADERS} ${AUDIO_SOURCES})

find_package(portaudio CONFIG REQUIRED)

target_include_directories(${target_name} 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
)

if(MSVC OR WIN32)
    target_link_libraries(${target_name} PRIVATE portaudio)
else()
    target_link_libraries(${target_name} PRIVATE portaudio_static)
endif()

target_link_libraries(${target_name}
    PRIVATE
        vad
        kernel
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