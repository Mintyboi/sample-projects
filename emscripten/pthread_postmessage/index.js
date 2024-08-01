var _consumer;
const msgChannelPorts = [];

const init = () => {
    _consumer = new Worker("main.js");

    _consumer.onmessage = workerOnMessage;
}

const workerOnMessage = (event) => {
    const data = event.data;
    if (data.cmd === "createMsgChannelPort") {
        // console.log(`[Main UI Thread] Channel ${data.workerID} Received port`);
        data.msgChannelPort.onmessage = msgChannelOnMessage;
        msgChannelPorts[data.workerID] = data.msgChannelPort;
    }
    else if (data.source === "appthread") {
        console.log(JSON.stringify(data));
        switch (data.type) {
            case "asyncCall":
                const userData = "Huge JSON object";
                console.log("2. [Main UI Thread] Received message from Application Thread");
                console.log(`3. [Main UI Thread] Sending back '${userData}' to Application Thread`);
                const msg = {
                    source: "mainthread",
                    eventType: "asyncCall",
                    userData: userData
                };
                if (data.workerID){
                    msgChannelPorts[data.workerID].postMessage(msg)
                }
                else {
                    _consumer.postMessage(msg);
                }
                break;
            default: {
                if (_onMessageCallback) {
                    _onMessageCallback(JSON.stringify(data));
                } else {
                    _msgCache.push(JSON.stringify(data));
                }
                break;
            }
        }
    }
}

const msgChannelOnMessage = (event) => {
    workerOnMessage(event);
}