# c2_switch.cmake - the recompiled tree and the SDK for the Condemned 2 build (from ds2-nx's ds2_switch.cmake).
#
#   C2_GEN_DIR   generated/default of tools/codegen.sh (made from YOUR default.xex; never committed)
#   REXSDK_DIR    this repository's sdk/ (rexglue-sdk-nx, with sdk-patches/ applied)
#
# The tree is compiled as a mirror of symlinks (tools/prepare_tree.py) whose PCH is the Switch one
# (tools/make_nx_pch.py: GCC's DEFINE_REX_FUNC, per-thread mtmsrd regions).

set(C2_GEN_DIR "" CACHE PATH "generated/default of tools/codegen.sh (holds sources.cmake)")
if(NOT EXISTS "${C2_GEN_DIR}/sources.cmake")
    message(FATAL_ERROR
        "C2_GEN_DIR must point at the codegen output of your default.xex "
        "(no sources.cmake under '${C2_GEN_DIR}'). Run tools/codegen.sh first.")
endif()

set(REXSDK_DIR "" CACHE PATH "Path to the rexglue-sdk-nx source tree")
if(NOT REXSDK_DIR)
    get_filename_component(REXSDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../sdk" ABSOLUTE)
endif()
add_subdirectory("${REXSDK_DIR}" rexglue-sdk)
message(STATUS "condemned2: ReXGlue SDK from ${REXSDK_DIR}")

find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
set(C2_TREE "${CMAKE_BINARY_DIR}/generated_nx")
execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/../tools/prepare_tree.py"
            "${C2_GEN_DIR}" "${C2_TREE}"
    RESULT_VARIABLE _cco_tree_rc)
if(NOT _cco_tree_rc EQUAL 0)
    message(FATAL_ERROR "condemned2: tools/prepare_tree.py failed (${_cco_tree_rc})")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${C2_GEN_DIR}/condemned2_pch.h"
    "${C2_GEN_DIR}/sources.cmake"
    "${CMAKE_CURRENT_LIST_DIR}/../tools/prepare_tree.py"
    "${CMAKE_CURRENT_LIST_DIR}/../tools/make_nx_pch.py")

set(REXGLUE_RECOMP_DEBUG_INFO "none" CACHE STRING
    "Debug info level for generated code: line-tables-only, full, or none")
set(REXGLUE_RECOMP_OPTIONS "")
if(REXGLUE_RECOMP_DEBUG_INFO STREQUAL "none")
    list(APPEND REXGLUE_RECOMP_OPTIONS $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-g0>)
endif()

include("${C2_TREE}/sources.cmake")
set(C2_GUEST_SOURCES ${GENERATED_SOURCES})
set_source_files_properties(${C2_GUEST_SOURCES} PROPERTIES COMPILE_OPTIONS "${REXGLUE_RECOMP_OPTIONS}")
# condemned2_register.cpp is one function of ~92,000 SetFunction calls, run once at boot: GCC at -O3 spent over an
# hour on it. Unoptimized it builds in minutes and costs nothing at run time.
set_property(SOURCE "${C2_TREE}/condemned2_register.cpp" APPEND PROPERTY COMPILE_OPTIONS -O0)
list(LENGTH C2_GUEST_SOURCES _cco_n)
message(STATUS "condemned2: ${_cco_n} recompiled sources from ${C2_GEN_DIR}")

# Usage: c2_setup_target(<target> [GPU_PLUGINS xenos])
macro(c2_setup_target target_name)
    add_library(${target_name}_recomp OBJECT ${C2_GUEST_SOURCES})
    target_include_directories(${target_name}_recomp PRIVATE "${C2_TREE}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(${target_name}_recomp PRIVATE rex::runtime)
    rexglue_apply_target_settings(${target_name}_recomp)
    target_precompile_headers(${target_name}_recomp PRIVATE "${C2_TREE}/condemned2_pch.h")
    target_compile_options(${target_name}_recomp PRIVATE ${REXGLUE_RECOMP_OPTIONS})
    target_link_libraries(${target_name} PRIVATE ${target_name}_recomp)
    target_include_directories(${target_name} PRIVATE "${C2_TREE}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(${target_name} PRIVATE rex::runtime)
    rexglue_configure_target(${target_name} ${ARGN})
endmacro()
