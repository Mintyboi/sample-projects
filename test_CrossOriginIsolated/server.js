const express = require("express");

var app = express();
app.use (function (req, res, next) {
    res.append("Cross-Origin-Embedder-Policy", "require-corp");
    res.append("Cross-Origin-Opener-Policy", "restrict-properties");
    next();
});

app.use(express.static(__dirname));

app.listen(8080, function () {
    console.log("\nServer Running on 8080");
});