cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(unit VERSION 1.0.1 LANGUAGES C CXX)

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_COMMIT_HASH)
    set(GIT_COMMIT_HASH "")
endif()

include(FetchContent)
include(CheckIPOSupported)
include(CheckCXXCompilerFlag)

check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT lto_error)

if(LTO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
else()
    message(STATUS "IPO/LTO not supported: ${lto_error}")
endif()

set(BUILD_SHARED_LIBS OFF)
set(NNL_ENABLE_LOGGING ON)


set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)


FetchContent_Declare(
    NNL
    GIT_REPOSITORY https://github.com/Raw-man/NNL.git
    GIT_TAG        a81c210
)

FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        08e0f7b2da3c27f76ebb0a92f1862e15070267cd
)

FetchContent_Declare(
    RANG
    GIT_REPOSITORY https://github.com/AleBiCi/rang.git
    GIT_TAG        f55210ce27c0fa87601526102904c2702256a8a5
)

set(BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(BUILD_TESTING OFF CACHE INTERNAL "" FORCE)
set(BUILD_PROGRAMS OFF CACHE INTERNAL "" FORCE)

FetchContent_Declare(
    sndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG        68f6c16fe1407eff4cdde158566694c3ed666c2f
)

FetchContent_Declare(
    samplerate
    GIT_REPOSITORY https://github.com/libsndfile/libsamplerate.git
    GIT_TAG        2ccde9568cca73c7b32c97fefca2e418c16ae5e3
)

FetchContent_MakeAvailable(NNL CLI11 RANG sndfile samplerate)

add_library(agauniyal::rang ALIAS rang)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

configure_file("${PROJECT_SOURCE_DIR}/include/unit_version.hpp.in"
    "${PROJECT_BINARY_DIR}/generated/include/unit_version.hpp")



add_compile_definitions(UNIT_BUILD_TYPE="${CMAKE_BUILD_TYPE}")


include_directories(
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated/include>
    )




set(Sources
src/main.cpp
src/app.cpp
src/app_setup_dnd.cpp
src/app_setup_general.cpp
src/app_setup_export.cpp
src/app_setup_import.cpp
src/app_setup_unpack.cpp
src/app_setup_pack.cpp
src/app_setup_md5.cpp
src/app_setup_locate.cpp
src/app_setup_detect.cpp

src/export.cpp
src/export_model.cpp
src/export_model_partial.cpp
src/export_minimap.cpp
src/export_collision_partial.cpp
src/export_shadow_collision_partial.cpp
src/export_dialog.cpp
src/export_dialog_partial.cpp
src/export_texture.cpp
src/export_posd.cpp
src/export_lit.cpp
src/export_fog.cpp
src/export_distance.cpp
src/export_pphd8.cpp

src/import.cpp
src/import_model.cpp
src/import_camera.cpp
src/import_position.cpp
src/import_minimap.cpp
src/import_fog.cpp
src/import_distance.cpp
src/import_light.cpp
src/import_dialog.cpp
src/import_sound.cpp
src/import_texture.cpp
src/unpack.cpp
src/pack.cpp
src/locate.cpp
src/detect.cpp

src/utils.cpp

)

add_executable(unit
      ${Sources}
)

target_link_libraries(unit PUBLIC Threads::Threads NNL::NNL CLI11::CLI11 agauniyal::rang SndFile::sndfile SampleRate::samplerate)
