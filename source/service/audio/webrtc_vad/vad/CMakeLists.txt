project(vad)

set(VAD_HEADERS 
    include/signal_processing_library.h
    include/spl_inl_armv7.h
    include/spl_inl_mips.h
    include/spl_inl.h
    include/vad_core.h
    include/vad_filterbank.h
    include/vad_gmm.h
    include/vad_sp.h
    include/vad.h
    include/webrtc_vad.h
)

set(VAD_SOURCES
    signal_processing_library.c
    vad_core.c
    vad_filterbank.c
    vad_gmm.c
    vad_sp.c
    vad.cc
    webrtc_vad.c
)

add_library(vad STATIC ${VAD_HEADERS} ${VAD_SOURCES})
target_include_directories(vad PUBLIC include)