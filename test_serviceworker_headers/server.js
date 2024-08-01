const express = require("express");

var app = express();
app.set('etag', true); // explicitly turning etag on

app.use(function (req, res, next) {  
    res.set('Cache-Control', 'public, max-age=3600');
    next();
})

app.use(express.static(__dirname));

app.listen(8080, function () {
    console.log("\nServer Running on 8080");
});