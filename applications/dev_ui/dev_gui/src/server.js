const express = require('express');
const path = require('path');
const app = express();

app.use(express.static(path.join(__dirname, '../build')));

app.get('*', function(req, res) {
    res.sendFile(path.join(__dirname, '../build', 'index.html'));
});

console.log("Starting developer GUI at port rasperry.host:3000");
app.listen(3000);