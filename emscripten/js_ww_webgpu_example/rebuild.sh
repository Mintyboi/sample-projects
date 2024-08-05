#!/bin/bash

if [ -d "build_web" ]; then
    rm -r build_web
    echo "deleting build_web generated folder"
fi

mkdir build_web
cp webgpuww.html build_web
cp main.js build_web

pushd build_web
emcc ../worker.cpp -s EXPORTED_FUNCTIONS=_render_on_offscreen_canvas -s ASSERTIONS=0 -s WASM=1 -s NO_EXIT_RUNTIME=1 --pre-js ../prejs.js --post-js ../postjs.js -s USE_WEBGPU=1 -o worker.js
popd

python3 -m http.server --directory "./build_web"