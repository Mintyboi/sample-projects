#!/bin/bash

if [ -d "build" ]; then
    rm -r build
    echo "deleting build generated folder"
fi

mkdir build
cp webgpuww.html build
cp main.js build

pushd build
emcc ../worker.cpp\
 -s EXPORTED_FUNCTIONS=_render_on_offscreen_canvas\
 -s ASSERTIONS=0\
 -s WASM=1\
 -s NO_EXIT_RUNTIME=1\
 --pre-js ../prejs.js\
 --post-js ../postjs.js\
 -s USE_WEBGPU=1\
 --js-library ../jslibrary.js\
 -s ASYNCIFY=2\
 -s ASYNCIFY_IMPORTS=jsTransferImageWebGPU\
 -s ASYNCIFY_EXPORTS=render_on_offscreen_canvas\
 -o worker.js
# -s ASYNCIFY=2\
#  -s ASYNCIFY_IMPORTS=jsTransferImageWebGPU\
#  -s ASYNCIFY_EXPORTS=_render_on_offscreen_canvas\
popd


python3 -m http.server --directory "./build"