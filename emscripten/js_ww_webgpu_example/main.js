let counter = 0;

const canvas = document.getElementById("canvas");
const context = canvas.getContext("2d");

// set up a worker thread to render to offscreen canvas
const worker = new Worker("worker.js");

function drawCanvas() {

    worker.postMessage({msgid: 1, cWidth: canvas.width, cHeight: canvas.height}, []);
}

drawCanvas();

// listen for myWorker to transfer the rendered image back to main
worker.addEventListener("message", function handleMessageFromWorker(msg) {

    // console.log("message from worker received in main:", msg);
    console.log("[main] received rendered image.. transferring image to HTML canvas");
    context.putImageData(msg.data.bm, 0, 0);
});