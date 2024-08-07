mergeInto(LibraryManager.library, {
    jsTransferImageWebGPU__sig: "v",
    jsTransferImageWebGPU: function() /* : void */ {
        console.log("JSLand: Transfer Image to offscreencanvas")
        return new Promise((resolve, reject) => {
            setTimeout(() => {
                console.log("JSLand: Coming back after settimeout")
                resolve();
            }, 1);
        });

        // For non-jspi
        // if (Module.stagingContext)
        // {
        //     Module.stagingContext.drawImage(Module.canvas, 0, 0);
        //     var stagingImgData = Module.stagingContext.getImageData(0, 0, Module.canvas.width, Module.canvas.height);
        //     if (stagingImgData)
        //     {
        //         console.log("[worker] extracted rendered image from offscreen canvas, posting to main");
        //         self.postMessage({ bm: stagingImgData }, []);
        //         stagingImgData = null;
        //     }
        // }
    }
  });