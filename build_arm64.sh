mkdir -p build_arm64 && cd build_arm64
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm64.cmake ..
cmake .
