cmake_minimum_required(VERSION 3.14)

project(template-project LANGUAGES ASM C CXX)
include(CTest)
enable_testing()

set(EXECUTABLE ${PROJECT_NAME}.elf)
set(BOARD_NAME "VCU" CACHE STRING "Board key from Core/Inc/Code_generation/JSON_ADE/boards.json" FORCE)
if(BOARD_NAME STREQUAL "")
  message(FATAL_ERROR "BOARD_NAME cannot be empty")
endif()
set(STLIB_DIR ${CMAKE_CURRENT_LIST_DIR}/deps/ST-LIB)
set(LD_SCRIPT ${STLIB_DIR}/STM32H723ZGTX_FLASH.ld)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(GENERATOR_SCRIPT "${CMAKE_SOURCE_DIR}/Core/Inc/Code_generation/Generator.py")

if(CMAKE_HOST_WIN32)
  set(VENV_PYTHON ${CMAKE_SOURCE_DIR}/virtual/Scripts/python)
else()
  set(VENV_PYTHON ${CMAKE_SOURCE_DIR}/virtual/bin/python)
endif()
if(EXISTS "${VENV_PYTHON}")
  set(PYTHON_FOR_TOOLS "${VENV_PYTHON}")
else()
  set(PYTHON_FOR_TOOLS "${Python3_EXECUTABLE}")
endif()

option(USE_ETHERNET "Enable ethernet peripheral" OFF)
option(TARGET_NUCLEO "Targets the STM32H723 Nucleo development board" OFF)
option(BUILD_EXAMPLES "Build Core/Src/Examples sources" OFF)
option(USE_CCACHE "Use ccache if available" ON)
if(NOT DEFINED ENABLE_LTO)
  if(CMAKE_CROSSCOMPILING)
    set(ENABLE_LTO OFF)
  else()
    set(ENABLE_LTO ON)
  endif()
endif()
set(ENABLE_LTO "${ENABLE_LTO}" CACHE BOOL "Enable link-time optimization (LTO)")

if(USE_CCACHE)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_ASM_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  endif()
endif()

message(STATUS "Template project: CMAKE_CROSSCOMPILING = ${CMAKE_CROSSCOMPILING}")
message(STATUS "Template project: USE_ETHERNET         = ${USE_ETHERNET}")
message(STATUS "Template project: TARGET_NUCLEO        = ${TARGET_NUCLEO}")
message(STATUS "Template project: BOARD_NAME           = ${BOARD_NAME}")

add_subdirectory(${STLIB_DIR})
message(STATUS "Using ST-LIB target: ${STLIB_LIBRARY}")

file(GLOB_RECURSE GENERATOR_JSONS
  CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/Core/Inc/Code_generation/JSON_ADE/*.json
)

set(GENERATED_DATA_PACKETS ${CMAKE_SOURCE_DIR}/Core/Inc/Communications/Packets/DataPackets.hpp)
set(GENERATED_ORDER_PACKETS ${CMAKE_SOURCE_DIR}/Core/Inc/Communications/Packets/OrderPackets.hpp)
add_custom_command(
  OUTPUT
  ${GENERATED_DATA_PACKETS}
  ${GENERATED_ORDER_PACKETS}
  COMMAND ${PYTHON_FOR_TOOLS} ${GENERATOR_SCRIPT} ${BOARD_NAME}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS
  ${GENERATOR_SCRIPT}
  ${CMAKE_SOURCE_DIR}/Core/Inc/Code_generation/Packet_generation/Packet_generation.py
  ${CMAKE_SOURCE_DIR}/Core/Inc/Code_generation/Packet_generation/DataTemplate.hpp
  ${CMAKE_SOURCE_DIR}/Core/Inc/Code_generation/Packet_generation/OrderTemplate.hpp
  ${GENERATOR_JSONS}
  COMMENT "Generating packets for ${BOARD_NAME}"
)

add_custom_target(run_generator
  DEPENDS
  ${GENERATED_DATA_PACKETS}
  ${GENERATED_ORDER_PACKETS}
)

file(GLOB_RECURSE METADATA_SOURCES
  CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/Core/*.c
  ${CMAKE_SOURCE_DIR}/Core/*.cpp
  ${CMAKE_SOURCE_DIR}/Core/*.h
  ${CMAKE_SOURCE_DIR}/Core/*.hpp
)

set(METADATA_OUTPUT ${CMAKE_SOURCE_DIR}/Core/Src/Runes/generated_metadata.cpp)
set(METADATA_TEMPLATE ${CMAKE_SOURCE_DIR}/tools/binary_metadata_template.cpp)
set(METADATA_SCRIPT ${CMAKE_SOURCE_DIR}/tools/generate_binary_metadata.py)

list(REMOVE_ITEM METADATA_SOURCES ${METADATA_OUTPUT})

add_custom_command(
  OUTPUT ${METADATA_OUTPUT}
  COMMAND ${PYTHON_FOR_TOOLS} ${METADATA_SCRIPT}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS
  ${METADATA_SCRIPT}
  ${METADATA_TEMPLATE}
  ${METADATA_SOURCES}
  COMMENT "Generating binary metadata"
)

add_custom_target(generate_binary_metadata
  DEPENDS ${METADATA_OUTPUT}
)

if(CMAKE_CROSSCOMPILING)
  file(GLOB_RECURSE SOURCE_C CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/Core/*.c)
  file(GLOB_RECURSE SOURCE_CPP CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/Core/*.cpp)
  file(GLOB_RECURSE SOURCE_H CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/Core/*.h)
  file(GLOB_RECURSE SOURCE_HPP CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/Core/*.hpp)

  if(NOT BUILD_EXAMPLES)
    file(GLOB_RECURSE EXAMPLE_CPP CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/Core/Src/Examples/*.cpp)
    list(REMOVE_ITEM SOURCE_CPP ${EXAMPLE_CPP})
  endif()

  add_executable(${EXECUTABLE}
    ${SOURCE_C}
    ${SOURCE_CPP}
    ${SOURCE_H}
    ${SOURCE_HPP}

    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:${STLIB_DIR}/startup_stm32h723zgtx.s>
  )

  target_link_libraries(${EXECUTABLE} PRIVATE
    ${STLIB_LIBRARY}
  )

  set_target_properties(${EXECUTABLE} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
  )

  target_compile_definitions(${EXECUTABLE} PRIVATE
    $<$<BOOL:${USE_ETHERNET}>:STLIB_ETH>
    $<IF:$<BOOL:${TARGET_NUCLEO}>,NUCLEO,BOARD>
    $<IF:$<BOOL:${TARGET_NUCLEO}>,HSE_VALUE=8000000,HSE_VALUE=25000000>
  )

  target_compile_options(${EXECUTABLE} PRIVATE
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mcpu=cortex-m7>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mfpu=fpv5-d16>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mfloat-abi=hard>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mthumb>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-specs=nosys.specs>
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:RelWithDebInfo>:-g>
    -ffunction-sections
    -fdata-sections
    -fno-exceptions

    -Wno-psabi

    $<$<COMPILE_LANGUAGE:C>:-w>

    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    # $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
    $<$<COMPILE_LANGUAGE:CXX>:-Werror>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  )

  target_include_directories(${EXECUTABLE} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
  )

  target_link_options(${EXECUTABLE} PRIVATE
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-T${LD_SCRIPT}>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mcpu=cortex-m7>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mthumb>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mfpu=fpv5-d16>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-mfloat-abi=hard>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-specs=nosys.specs>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-lc>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-lm>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-lnosys>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-Wl,-Map=${PROJECT_NAME}.map,--cref>
    $<$<BOOL:${CMAKE_CROSSCOMPILING}>:-Wl,--gc-sections>
  )

  add_dependencies(${EXECUTABLE} run_generator generate_binary_metadata)

  if(ENABLE_LTO)
    set_property(TARGET ${EXECUTABLE} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set_property(TARGET ${EXECUTABLE} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
  endif()

  # Post-build: Copy binary to out/build/latest.elf and create marker for BOARD builds
  add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    # Create output directory
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${CMAKE_SOURCE_DIR}/out/build
    # Copy the built binary to out/build/latest.elf
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${EXECUTABLE}>
    ${CMAKE_SOURCE_DIR}/out/build/latest.elf
    COMMENT "Copying ${EXECUTABLE} to out/build/latest.elf"
  )

  # Create or remove marker file for BOARD builds (used by preflash_check.py)
  if(NOT TARGET_NUCLEO)
    add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_SOURCE_DIR}/out/build/board_build_marker
      COMMENT "Creating BOARD build marker for pre-flash check"
    )
  else()
    add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/out/build/board_build_marker
      COMMENT "Removing BOARD build marker (NUCLEO build)"
    )
  endif()
endif()

if(PROJECT_IS_TOP_LEVEL)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
endif()
