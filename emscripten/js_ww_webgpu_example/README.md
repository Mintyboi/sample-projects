# js_ww_webgpu_example
This repository demonstrates creation of web worker from main JS, performs webgpu rendering on worker thread and rendered output is passed to main thread for display on canvas.
Took from original version in https://git.autodesk.com/lowl/js_compute_webgpu_example

## How to rebuild and host static HTML page
```
./rebuild.sh
```

## How to update emsdk version for this example
Follow instructions in this page: https://emscripten.org/docs/getting_started/downloads.html
```
./emsdk update
./emsdk install 2.0.32
./emsdk activate 2.0.32
./emcc -v
```

## HTML render output
![RenderOutputV2](https://media.git.autodesk.com/user/2916/files/556a92d0-14da-4f67-9ab7-842e976e8033)

## Last tested working on
```
env: wsl
emsdk version: 3.1.20
ubuntu: 20.04
```
