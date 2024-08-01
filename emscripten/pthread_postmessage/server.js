const express = require("express");
const http = require('http');

express.static.mime.types['wasm'] = 'application/wasm';
var app = express();

app.use (function (req, res, next) {
    res.append('Cross-Origin-Embedder-Policy', 'require-corp');
    res.append('Cross-Origin-Opener-Policy', 'same-origin');
    
    next();
});

app.use('/', express.static(__dirname));
app.get('/*', function (req, res, next)
{
    res.header('Service-Worker-Allowed', '/');
    next(); // http://expressjs.com/guide.html#passing-route control
});

const port = 9999
http.createServer(app).listen(port, function () {
    console.log(`\nServer Running on ${port}`);
    console.log(`Available on:
  http://localhost:${port}
  http://127.0.0.1:${port}
Hit CTRL-C to stop the server`);
});

process.on('SIGINT', function () {
    console.log('http-server stopped.');
    process.exit();
  });

process.on('SIGTERM', function () {
    console.log('http-server stopped.');
    process.exit();
});
