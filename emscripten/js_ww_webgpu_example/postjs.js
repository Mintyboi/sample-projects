// Following code section is generally copied from JS generated with BUILD_AS_WORKER flag
// turned on.
// This code section is required for _render_on_offscreen_canvas() defined in worker.wasm to
// be loaded before calling it from onmessage()
(function () {
    var messageBuffer = null, buffer = 0, bufferSize = 0;

    function flushMessages() {
        if (!messageBuffer) return;
        if (runtimeInitialized) {
            var temp = messageBuffer;
            messageBuffer = null;
            temp.forEach(function (message) {
                onmessage(message);
            });
        }
    }

    function messageResender() {
        flushMessages();
        if (messageBuffer) {
            setTimeout(messageResender, 100); // still more to do
        }
    }

    function drawOnOffscreenCanvas() {

        while (1)
        {
            console.log("[worker] received offscreen canvas.. rendering...");
            _render_on_offscreen_canvas();

            // debugger;

            if (Module.stagingContext)
            {
                Module.stagingContext.drawImage(Module.canvas, 0, 0);
                var stagingImgData = Module.stagingContext.getImageData(0, 0, Module.canvas.width, Module.canvas.height);
                if (stagingImgData)
                {
                    console.log("[worker] extracted rendered image from offscreen canvas, posting to main");
                    self.postMessage({ bm: stagingImgData }, []);
                    stagingImgData = null;
                }
            }
        }
    }

    onmessage = (msg) => {
        // if main has not yet been called (mem init file, other async things), buffer messages
        if (!runtimeInitialized) {
            if (!messageBuffer) {
                messageBuffer = [];
                setTimeout(messageResender, 100);
            }
            messageBuffer.push(msg);
            return;
        }
        flushMessages();

        // var func = Module['_' + msg.data['funcName']];
        // if (!func) throw 'invalid worker function to call: ' + msg.data['funcName'];
        // var data = msg.data['data'];
        // if (data) {
        //   if (!data.byteLength) data = new Uint8Array(data);
        //   if (!buffer || bufferSize < data.length) {
        //     if (buffer) _free(buffer);
        //     bufferSize = data.length;
        //     buffer = _malloc(data.length);
        //   }
        //   HEAPU8.set(data, buffer);
        // }

        // workerResponded = false;
        // workerCallbackId = msg.data['callbackId'];
        // if (data) {
        //   func(buffer, data.length);
        // } else {
        //   func(0, 0);
        // }

        // ----------------------------------------------------

        // === MODIFIED ===
        // console.log("message from main received in worker:", msg);
        if (msg.data.msgid == 1) {

            Module.canvas = new OffscreenCanvas(msg.data.cWidth, msg.data.cHeight);
            var stagingCanvas = new OffscreenCanvas(msg.data.cWidth, msg.data.cHeight);
            Module.stagingContext = stagingCanvas.getContext("2d", {willReadFrequently: true});
            specialHTMLTargets["!canvas"] = Module.canvas;
        }

        drawOnOffscreenCanvas();

        debugger;
        console.log("after drawOnOffscreenCanvas(), should not reach here!");
    }
})();
