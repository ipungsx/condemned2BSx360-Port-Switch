# condemned2 - Condemned 2: Bloodshot (Xbox 360) on the Nintendo Switch, as a homebrew NRO.
#
# Uses condemned-nx (Condemned 1) and ds2-nx as a reference for the app shell. No recompiled tree of this game
# exists anywhere: tools/codegen.sh generates it from YOUR default.xex with ReXGlue v0.10.0 (recomp/).
#
# Configure (Switch; tools/build_linux.sh does all of this):
#   cmake -S condemned2 -B condemned2/out/sw -G Ninja -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_TOOLCHAIN_FILE=tools/switch/cmake/switch-devkitA64.cmake \
#     -DREXSDK_DIR=$PWD/sdk -DC2_GEN_DIR=$PWD/condemned2/recomp/generated/default -DREXGLUE_SWITCH_DEKO3D=ON
#   cmake --build condemned2/out/sw

cmake_minimum_required(VERSION 3.25)
project(condemned2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/c2_switch.cmake)

set(C2_SOURCES
    src/main.cpp
    src/c2_cpu.cpp
    src/c2_shader_dump.cpp
    src/c2_shaders.cpp
    src/c2_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shaders/ds_shader_library.cpp
)
if(REXGLUE_SWITCH_DEKO3D)
    list(APPEND C2_SOURCES src/c2_gpu.cpp)
endif()

add_executable(condemned2 ${C2_SOURCES})

if(REXGLUE_SWITCH_DEKO3D)
    c2_setup_target(condemned2)
    rexglue_switch_add_deko3d(condemned2)
    target_compile_definitions(condemned2 PRIVATE C2_DEKO3D=1)
else()
    c2_setup_target(condemned2 GPU_PLUGINS xenos)
endif()

target_include_directories(condemned2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../shaders
                           ${REXSDK_DIR}/thirdparty/xxHash)

set(C2_VERSION "0.2.1")
execute_process(COMMAND git rev-parse --short=10 HEAD WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE C2_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT C2_COMMIT)
    set(C2_COMMIT "unknown")
endif()
target_compile_definitions(condemned2 PRIVATE C2_VERSION="${C2_VERSION}" C2_COMMIT="${C2_COMMIT}")

# Debug modes (ZL + ZR shortcuts, telemetria.flag) are ON while the port is being brought up.
option(C2_DEPURACION "Debug modes: ZL + ZR shortcuts, telemetria.flag" ON)
target_compile_definitions(condemned2 PRIVATE C2_DEPURACION=$<BOOL:${C2_DEPURACION}>)

if(REXGLUE_PLATFORM_SWITCH)
    # Homebrew Menu icon: the game's own title image from YOUR default.xex when tools/make_icon.py has made
    # assets/icon.jpg (never committed); otherwise res/icon.jpg, ours, with nothing from the game.
    option(C2_ICON_DEL_DISCO "Homebrew menu icon from your own disc (assets/icon.jpg)" ON)
    set(C2_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.jpg")
    if(C2_ICON_DEL_DISCO AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpg")
        set(C2_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpg")
    endif()
    message(STATUS "condemned2: icon ${C2_ICON}")
    rexglue_switch_add_nro(condemned2
        NAME "Condemned 2: Bloodshot"
        AUTHOR "ipungsx"
        VERSION "${C2_VERSION}"
        ICON "${C2_ICON}")
endif()
