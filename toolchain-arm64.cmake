
set(CMAKE_TOOLCHAIN_FILE "toolchain-arm64.cmake")
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR arm64)
SET(CMAKE_SYSTEM_VERSION 1)

# Specify the cross compiler
# clone rpi tools from https://github.com/raspberrypi/tools.git
#set(tools <path-to-raspberrypi-tools>/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf)
SET(CMAKE_C_COMPILER aarch64-unknown-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER aarch64-unknown-linux-gnu-g++)

# Where is the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/aarch64-unknown-linux-gnu)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --sysroot=${CMAKE_FIND_ROOT_PATH}")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --sysroot=${CMAKE_FIND_ROOT_PATH}")
SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --sysroot=${CMAKE_FIND_ROOT_PATH}")

# Search for programs only in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers only in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(ARM64 1)
