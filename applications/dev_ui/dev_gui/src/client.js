const express = require('express');
const path = require('path');
const https = require('https');
const http = require('http');
const fs = require('fs');

const httpsPort = 3000;
const httpPort = 3080;
const app = express();

app.use(express.static(path.join(__dirname, '../build')));

app.get('*', function(req, res) {
    res.sendFile(path.join(__dirname, '../build', 'index.html'));
});

try {
    http.createServer(app).listen(httpPort, () => {
        console.log("Developer GUI http client starting on port : " + httpPort);
    });
    if (process.env && process.env.HTTPS && process.env.SSL_CRT_FILE && process.env.SSL_KEY_FILE) {
        let options = {
            key: fs.readFileSync(process.env.SSL_KEY_FILE),
            cert: fs.readFileSync(process.env.SSL_CRT_FILE)
        };

        https.createServer(options, app).listen(httpsPort, () => {
            console.log("Developer GUI https client starting on port : " + httpsPort);
        });
    }
} catch (e) {
    console.log(`Could not start client: ${e.message}`);
    return;
}