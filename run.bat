@echo off

set example=%1%
pushd res
start ../build/%example%.exe
popd ..
