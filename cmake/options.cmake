

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_TESTING "Enable testing" ON)


if(BUILD_SHARED_LIBS)
    message(STATUS "Building shared libraries")
    set(TARGET_LIB_TYPE SHARED)
else()
    message(STATUS "Building static libraries")
    set(TARGET_LIB_TYPE STATIC)
endif()

option(BUILD_FILE_SERVICE "Build file service" OFF)
option(BUILD_GAME_SERVICE "Build game service" OFF)
option(BUILD_MUSIC_SERVICE "Build music service" OFF)
option(BUILD_NOVEL_SERVICE "Build novel service" OFF)
option(BUILD_VIDEO_SERVICE "Build video service" OFF)
option(BUILD_WEB_SERVICE "Build web service" OFF)
option(BUILD_WRITER_SERVICE "Build writer service" OFF)

option(BUILD_AI_PROVIDERS "Build AI providers extension" ON)
option(BUILD_WEATHER_PROVIDERS "Build weather providers extension" OFF)
