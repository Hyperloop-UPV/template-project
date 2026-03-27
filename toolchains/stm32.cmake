set(_stlib_stm32_toolchain "${CMAKE_CURRENT_LIST_DIR}/../deps/ST-LIB/toolchains/stm32.cmake")

if(NOT EXISTS "${_stlib_stm32_toolchain}")
    message(FATAL_ERROR
        "Missing shared STM32 toolchain file:\n  ${_stlib_stm32_toolchain}\n"
        "Initialize deps/ST-LIB before configuring the MCU preset."
    )
endif()

include("${_stlib_stm32_toolchain}")
