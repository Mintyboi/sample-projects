https://github.com/emscripten-core/emscripten/issues/18108

Steps to reproduce:

1. Set up emscripten environment
2. Call build.bat
3. Copy package.json and server.js into dist folder
4. call npm install
5. call npm start
6. Open browser dev console and navigate to localhost:9999

Results:
```
1. [Application Thead] ENVIRONMENT_IS_PTHREAD = false; Making async call; Posting message to Main UI thread;
2. [Main UI Thread] Received message from Application Thread
3. [Main UI Thread] Sending back 'Huge JSON object' to Application Thread
4. [Application Thead] ENVIRONMENT_IS_PTHREAD = false; Received 'Huge JSON object' from Main UI thread; ==== LOOP SUCCESS
1. [Application Thead] ENVIRONMENT_IS_PTHREAD = true; Making async call; Posting message to Main UI thread;
x. [Application Thread] ENVIRONMENT_IS_PTHREAD = false We should not be receiving message from another worker
still waiting on run dependencies:
dependency: wasm-instantiate
(end of list)
still waiting on run dependencies:
dependency: __asyncCall__
(end of list)
...
...
```