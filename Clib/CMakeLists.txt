cmake_minimum_required(VERSION 3.20)
# Generic makes it cross Platfom compiling
set(CMAKE_SYSTEM_NAME Generic)
# Browser sandbox env is 32 bit
set(CMAKE_SYSTEM_PROCESSOR wasm32)
# Avoid CMake trying to do execute tests and only check for compile
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project (Clib C)
# C flags
# adding flag for optimization: LTO and O2 
# adding warning flags
add_compile_options(--target=wasm32 -O2 -flto -nostdlib -Wall -Wextra)
#LD Flags, using llvm linker, and set the memory 
#--export=memory exposed the linear memory to host (JS) env
add_link_options(
    --target=wasm32
    -nostdlib
    -fuse-ld=lld 
    -Wl,--no-entry
    -Wl,--allow-undefined
    -Wl,--export=memory
    -Wl,--export-dynamic
)

file(GLOB SRC_FILES "*.c")
# the final output name must be clib.wasm, and output to client/public
add_executable(clib ${SRC_FILES})
set_target_properties(clib PROPERTIES SUFFIX ".wasm")
set_target_properties(clib PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../client/public")