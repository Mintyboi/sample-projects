@ECHO OFF
del dist\main.*
call emcc main.cpp -s PROXY_TO_PTHREAD --profiling -pthread -s PTHREAD_POOL_SIZE=1 -s ALLOW_BLOCKING_ON_MAIN_THREAD=0 -sEXIT_RUNTIME=0 --pre-js preload.js -s INITIAL_MEMORY=1GB -o dist\main.js

copy index.* dist