@echo off

SET build_type=Release
SET cmake_flags=""

@REM Check first argument
if /i "%1" EQU "debug" (
    SET build_type=Debug
    SHIFT
)

IF NOT EXIST build mkdir build
pushd build
DEL *.pdb >nul 2>&1
cmake -DCMAKE_BUILD_TYPE=%build_type% -DALCHEMY_INCLUDE_EXAMPLES=ON ..
cmake --build .
popd
