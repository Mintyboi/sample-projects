let _onAsyncCallResolve;

const execAsyncCall = () => {
    return new Promise((resolve) => {
        _onAsyncCallResolve = resolve;
        const msg = { source: "appthread", type: "asyncCall" };
        if (ENVIRONMENT_IS_PTHREAD && Module['msgChannelPort']) {
            msg.workerID = Module['workerID'];
            Module['msgChannelPort'].postMessage(msg);
        }
        else {
            postMessage(msg);
        }
    });
}
const onAsyncCallComplete = (userData) => {
    _onAsyncCallResolve && _onAsyncCallResolve(userData);
}

const asyncCall = () => {
    addRunDependency("__asyncCall__");
    console.log(`1. [Application Thead] ENVIRONMENT_IS_PTHREAD = ${ENVIRONMENT_IS_PTHREAD}; Making async call; Posting message to Main UI thread;`);
    execAsyncCall().then((userData) => {
        console.log(`4. [Application Thead] ENVIRONMENT_IS_PTHREAD = ${ENVIRONMENT_IS_PTHREAD}; Received '${userData}' from Main UI thread; ==== LOOP SUCCESS`);
        removeRunDependency("__asyncCall__");
    });
}

onmessage = (message) => {
    const data = message.data;
    switch (data.source) {
        case "mainthread": {
            if (data.eventType === "asyncCall") {
                onAsyncCallComplete(data.userData);
            }
            break;
        }
    }
    if (Module["PThreadOnMessage"]){
        Module["PThreadOnMessage"](message)
    }
}

Module.instantiateWasm = function (imports, successCallback) {
    asyncCall();

    if (ENVIRONMENT_IS_PTHREAD) {
        // Instantiate from the module posted from the main thread.
        // We can just use sync instantiation in the worker.
        var instance = new WebAssembly.Instance(Module['wasmModule'], imports);
        // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193,
        // the above line no longer optimizes out down to the following line.
        // When the regression is fixed, we can remove this if/else.
        successCallback(instance);
        // We don't need the module anymore; new threads will be spawned from the main thread.
        Module['wasmModule'] = null;
        return instance.exports;
    }
    else {
        // browser supports instantiate Streaming
        if (typeof WebAssembly.instantiateStreaming == "function") {
            WebAssembly.instantiateStreaming(fetch("main.wasm"), imports)
            .then(obj => {
                successCallback(obj.instance, obj.module);
            })
            .catch(function (reason) {
                abort(reason);
            });
        }
    }
}

Module.msgChannelOnMessage = (event) => {
    // console.log(`[AppThread] received from Frontend; ${JSON.stringify(event['data'])}`);
    onmessage(event);
}