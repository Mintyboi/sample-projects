@ECHO OFF
set pwd=%~dp0
set config=build
set output_path= "%pwd%/%config%"
rmdir /s/q %output_path%
md %output_path%

:: Build the side module
call emcc^
 side.cpp^
 -O1^
 -s WASM=1^
 -s SIDE_MODULE=1^
 -s ASSERTIONS=1^
 -s DEMANGLE_SUPPORT=1^
 -s DISABLE_EXCEPTION_CATCHING=0^
 -s BINARYEN=1^
 -s BINARYEN_METHOD='native-wasm'^
 -s ERROR_ON_UNDEFINED_SYMBOLS=0^
 -o %config%\side.wasm

:: Build the main module
call emcc^
 main.cpp^
 -O1^
 --preload-file %output_path%/side.wasm@side.wasm^
 -s WASM=1^
 -s MAIN_MODULE=2^
 -s EXPORTED_FUNCTIONS=@exports.json^
 -o %config%\index.html^
