@echo off

call build.bat
pushd res
start ../build/alchemy.exe
popd ..
