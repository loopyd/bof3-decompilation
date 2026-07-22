cmake_minimum_required(VERSION 3.10)
project(bof3-audio C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(psx_audio STATIC
    psx_adpcm.c
    psx_wav.c
    psx_xa.c
    psx_vab.c
    psx_sep.c
    psx_spu.c
    psx_render.c
)
target_include_directories(psx_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(bof3-audio main.c)
target_link_libraries(bof3-audio psx_audio)
target_include_directories(bof3-audio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

find_library(MATH_LIB m)
if(MATH_LIB)
    target_link_libraries(bof3-audio ${MATH_LIB})
endif()

if(WIN32)
    target_link_libraries(bof3-audio winmm ole32)
elseif(APPLE)
    find_library(COREAUDIO CoreAudio)
    find_library(AUDIOUNIT AudioUnit)
    find_library(COREFOUNDATION CoreFoundation)
    target_link_libraries(bof3-audio ${COREAUDIO} ${AUDIOUNIT} ${COREFOUNDATION})
else()
    find_package(Threads REQUIRED)
    target_link_libraries(bof3-audio Threads::Threads ${CMAKE_DL_LIBS})
    find_library(ALSA_LIB asound)
    if(ALSA_LIB)
        target_link_libraries(bof3-audio ${ALSA_LIB})
    endif()
endif()
