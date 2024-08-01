const express = require("express");

var app = express();
app.use (function (req, res, next) {
    res.append("Cross-Origin-Embedder-Policy", "require-corp");
    res.append("Cross-Origin-Opener-Policy", "restrict-properties");
    res.append("Origin-Trial", "AlA7oG4Y1bFl42WV2AiMsmLi/0Lxq2VuziZBsD4iP6l8DhuKAItVlG02s2CRekw0A+hoG2e7VQnHO8STISiQCAEAAABZeyJvcmlnaW4iOiJodHRwOi8vbG9jYWxob3N0OjgwODAiLCJmZWF0dXJlIjoiQ29vcFJlc3RyaWN0UHJvcGVydGllcyIsImV4cGlyeSI6MTcwNzI2Mzk5OX0=");
    next();
});

app.use(express.static(__dirname));

app.listen(8080, function () {
    console.log("\nServer Running on 8080");
});