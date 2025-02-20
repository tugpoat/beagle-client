mkdir -p build_x86 && cd build_x86
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
cp ../config.ini.sample ./config.ini
