@echo off

IF NOT EXIST build mkdir build
pushd build
DEL *.pdb >nul 2>&1
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j
popd
