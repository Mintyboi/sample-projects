if (navigator["gpu"]) {
    navigator["gpu"]["requestAdapter"]().then(function (adapter) {
        adapter["requestDevice"]().then(function (device) {
            Module["preinitializedWebGPUDevice"] = device;
            console.log("-------- device initialized");
        });
    }, function () {
        console.error("No WebGPU adapter; not starting");
    });
} else {
    console.error("No support for WebGPU; not starting");
}