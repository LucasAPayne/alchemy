@echo off

set startup=%1
IF NOT EXIST build mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=Debug -DSTARTUP:STRING=%startup% ..
cmake --build . 
popd
