@echo off

IF NOT EXIST build mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . 
popd
