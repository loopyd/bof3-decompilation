cmake_minimum_required(VERSION 3.10)
project(bof3-audio C)

include(CTest)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(psx_audio STATIC
    audio_render.c
    adpcm.c
    wav.c
    ogg.c
    flac.c
    game_render.c
    psf.c
    psx_machine.c
    xa.c
    vab.c
    sep.c
    spu.c
    spu_device.c
    emi.c
    render.c
    sf2.c
)
target_include_directories(psx_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(ZLIB REQUIRED)
target_link_libraries(psx_audio PUBLIC ZLIB::ZLIB)

find_library(VORBISENC_LIB vorbisenc)
find_library(VORBIS_LIB vorbis)
find_library(OGG_LIB ogg)
if(VORBISENC_LIB AND VORBIS_LIB AND OGG_LIB)
    target_compile_definitions(psx_audio PRIVATE HAVE_VORBIS=1)
    target_link_libraries(psx_audio PUBLIC ${VORBISENC_LIB} ${VORBIS_LIB} ${OGG_LIB})
endif()

find_library(FLAC_LIB FLAC)
if(FLAC_LIB)
    target_compile_definitions(psx_audio PRIVATE HAVE_FLAC=1)
    target_link_libraries(psx_audio PUBLIC ${FLAC_LIB})
endif()

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

if(BUILD_TESTING)
    add_executable(psf_test psf_test.c)
    target_link_libraries(psf_test psx_audio)
    add_test(NAME psf_test COMMAND psf_test)
    add_executable(psx_machine_test psx_machine_test.c)
    target_link_libraries(psx_machine_test psx_audio)
    add_test(NAME psx_machine_test COMMAND psx_machine_test)
    add_executable(audio_core_test audio_core_test.c)
    target_link_libraries(audio_core_test psx_audio)
    add_test(NAME audio_core_test COMMAND audio_core_test)
    add_executable(spu_device_test spu_device_test.c)
    target_link_libraries(spu_device_test psx_audio)
    add_test(NAME spu_device_test COMMAND spu_device_test)
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
