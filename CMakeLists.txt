# Set the minimum version of CMake that can be used
cmake_minimum_required(VERSION 3.20)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Set target definition variables required by the toolchain file
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/target.cmake)

# Set the toolchain file for cross-compilation
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)

# Project definition
project(VarioPrototype LANGUAGES C ASM)

set(PROJECT_NAME VarioPrototype)

# Create an executable object type
add_executable(${PROJECT_NAME})

# Include additional CMake files for flags, components, and files
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/flags.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/components.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/files.cmake)

# Add sources to executable/library
target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/startup_stm32f407xx.S"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/hal/gpio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/hal/i2c.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/drivers/ms5611.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/fault_handlers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/event_groups.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/queue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/stream_buffer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/tasks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/timers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS/portable/GCC/ARM_CM4F/port.c"
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
  third_party
  third_party/FreeRTOS/include
  third_party/FreeRTOS/portable/GCC/ARM_CM4F
)

set(COMMON_WARNINGS
    -Wall -Wextra -Wpedantic -Werror
    -Wconversion -Wshadow -Wimplicit-fallthrough
    -Wundef -Wdouble-promotion -Wconversion
)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
  # User defined compiler options
  $<$<COMPILE_LANGUAGE:C>:${COMMON_WARNINGS}>
  $<$<CONFIG:Release>:-Os>
)

# Add additional defined symbols
target_compile_definitions(${PROJECT_NAME} PRIVATE
  # Add additional defined symbols here
)

# Linker options
target_link_options(${PROJECT_NAME} PRIVATE 
  # User defined linker options
  -nostartfiles
)

# Add additional linker search directories
target_link_directories(${PROJECT_NAME} PRIVATE
  # Add additional linker search directories here
)

# Add additional linker libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
  # Add additional linker libraries here
)

set(FREERTOS_SOURCES
    third_party/FreeRTOS/tasks.c
    third_party/FreeRTOS/list.c
    third_party/FreeRTOS/queue.c
    third_party/FreeRTOS/timers.c
    third_party/FreeRTOS/event_groups.c
    third_party/FreeRTOS/stream_buffer.c
    third_party/FreeRTOS/portable/GCC/ARM_CM4F/port.c
)

set_source_files_properties(${FREERTOS_SOURCES} PROPERTIES
    COMPILE_OPTIONS "-w"
)