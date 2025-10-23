project(webrtc_vad)

add_subdirectory(vad)

if(BUILD_EXSAMPLES)
    add_subdirectory(examples)
endif()