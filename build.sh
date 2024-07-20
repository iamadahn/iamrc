mkdir -p build
cd build
cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=1
cmake --build .
