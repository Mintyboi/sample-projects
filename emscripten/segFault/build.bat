@ECHO OFF
set 'EMCC_FORCE_STDLIBS' = 'libc,libc++,libc++abi'
set BOOST_INCLUDE_PATH=../boost-headers/Inc/boost-1_57
set BOOST_BC_PATH=../boost/boost.bc

:: Segmentation fault with flto
::call emcc main.cpp %BOOST_BC_PATH% -I%BOOST_INCLUDE_PATH% -std=c++17 -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR -s MAIN_MODULE=2 -s SAFE_HEAP=1 -s NO_EXIT_RUNTIME=1 -s DISABLE_EXCEPTION_CATCHING=0 --profiling -flto 

:: No segmentation fault without flto
call emcc main.cpp %BOOST_BC_PATH% -I%BOOST_INCLUDE_PATH% -std=c++17 -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR -s MAIN_MODULE=2 -s SAFE_HEAP=1 -s NO_EXIT_RUNTIME=1 -s DISABLE_EXCEPTION_CATCHING=0 --profiling


