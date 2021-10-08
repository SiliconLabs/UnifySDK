const express = require('express');
const path = require('path');
const https = require('https');
const fs = require('fs');

const port = 3000;
const app = express();

app.use(express.static(path.join(__dirname, '../build')));

app.get('*', function(req, res) {
    res.sendFile(path.join(__dirname, '../build', 'index.html'));
});

if (process.env && process.env.HTTPS && process.env.SSL_CRT_FILE && process.env.SSL_KEY_FILE) {
    try {
        let options = {
            key: fs.readFileSync(process.env.SSL_KEY_FILE),
            cert: fs.readFileSync(process.env.SSL_CRT_FILE)
        };

        var server = https.createServer(options, app);
        server.listen(port, () => {
            console.log("Developer GUI https server starting on port : " + port);
        });

    } catch (e) {
        console.log(`Could not start server: ${e.message}`);
        return;
    }
} else {
    app.listen(port, () => {
        console.log("Developer GUI http server starting on port : " + port);
    });
}