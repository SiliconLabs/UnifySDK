"use strict";

let handler = require('./handler.js');
let webSocketServer = require('websocket').server;
let https = require('https');
let http = require('http');
let mqtt = require('mqtt');
let fs = require('fs');
let os = require('os');
let supportedClusters = require('../cluster-types/supported-clusters.js').SupportedClusters;

process.title = 'dev-gui-api';
const webSocketsServerPort = 1337;
const webSocketsSecureServerPort = 1343;

let mqttConnection;
let mqttConnectionProps = {
    Host: "",
    Port: 0,
    UseTLS: false
};

let topics = ["ucl/SmartStart/List",
    "ucl/SmartStart/CommissionableDevice/#",
    "ucl/by-unid/+/State",
    "ucl/by-unid/+/State/SupportedCommands",
    "ucl/by-unid/+/ProtocolController/NetworkManagement",
    "ucl/by-unid/+/ProtocolController/RFTelemetry/#",
    "ucl/by-unid/+/+/Groups/#",
    "ucl/by-unid/+/+/OTA/#",
    "ucl/OTA/info/+/+",
    "ucl/by-group/#",
    "ucl/UPTICap/#",
    "ucl/by-machine-id/+/SystemMetrics/SupportedCommands",
    "ucl/by-machine-id/+/SystemMetrics/Attributes/#",
    "ucl/by-mqtt-client/+/ApplicationMonitoring/Attributes/#"
];
Object.keys(supportedClusters).forEach((i) => {
    topics.push(`ucl/by-unid/+/+/${i}/SupportedCommands`);
    topics.push(`ucl/by-unid/+/+/${i}/Attributes/#`);
    topics.push(`ucl/by-unid/+/+/${i}/GeneratedCommands/IQReport`);
    topics.push(`ucl/by-unid/+/+/${i}/GeneratedCommands/AngleReport`);
});
console.log(`TopicList:${JSON.stringify(topics)}`);

if (process.env && process.env.HTTPS !== undefined && JSON.parse(process.env.HTTPS) && process.env.SSL_CRT_FILE && process.env.SSL_KEY_FILE) {
    try {
        console.log("Certificates:" + JSON.stringify(getCertificates()));
        let options = {
            cert: fs.readFileSync(process.env.SSL_CRT_FILE),
            key: fs.readFileSync(process.env.SSL_KEY_FILE)
        };
        let secureServer = https.createServer(options);
        secureServer.listen(webSocketsSecureServerPort, function() {
            console.log(`${new Date()}: API is listening on wss://${os.hostname}:${webSocketsSecureServerPort}`);
        });

        (new webSocketServer({ httpServer: secureServer })).on('request', onRequest);

    } catch (e) {
        console.log(`Could not start https api: ${e.message}`);
        process.exit(1);
    }
}

try {
    let server = http.createServer();
    server.listen(webSocketsServerPort, function() {
        console.log(`${new Date()}: API is listening on ws://${os.hostname}:${webSocketsServerPort}`);
    });

    (new webSocketServer({ httpServer: server })).on('request', onRequest);
} catch (e) {
    console.log(`Could not start https api: ${e.message}`);
    process.exit(1);
}

handler.runQueueHandler();

function onRequest(request) {
    let connection = request.accept(undefined, request.origin);
    let index = handler.clients.push(connection) - 1;
    console.log(`${new Date()}: Connection WebSocket accepted`);
    if (mqttConnection && mqttConnection.connected == true)
        handler.sendToAll(handler.getInitState(mqttConnectionProps));
    handler.sendToAll({ type: "certificates", data: getCertificates() });

    connection.on('message', function(message) {
        let mes = {};
        try {
            if (!message.type || message.type !== "utf8" || !message.utf8Data)
                return;
            mes = JSON.parse(message.utf8Data);
            if (mes.type === "fileUpload") {
                saveFile(mes);
                return;
            }
        } catch {
            console.log(`Could not parse request data: '${JSON.stringify(message)}'`);
        }
        try {
            switch (mes.type) {
                case "connect":
                    console.log("Connecting to MQTT");
                    connect(mes.data);
                    break;
                case "disconnect":
                    mqttConnection.end();
                    break;
                case "get-smart-start-list":
                    console.log("Retrieve SmartStart List");
                    let response = {
                        type: "smart-start-list",
                        data: handler.state.SmartStart
                    };
                    handler.sendToAll(response);
                    break;
                case "run-smart-start-command":
                    console.log(`Run SmartStart Command: ucl/SmartStart/List/${mes.data.Cmd} ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/SmartStart/List/${mes.data.Cmd}`, JSON.stringify(mes.data.Payload));
                    break;
                case "run-state-command":
                    console.log(`Run Node Command: ucl/by-unid/${mes.data.Unid}/ProtocolController/NetworkManagement/Write ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/ProtocolController/NetworkManagement/Write`, JSON.stringify(mes.data.Payload));
                    break;
                case "run-node-command":
                    console.log(`Run State Command: ucl/by-unid/${mes.data.Unid}/State/Commands/${mes.data.Payload} {}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/State/Commands/${mes.data.Payload}`, "{}");
                    break;
                case "run-cluster-command":
                    console.log(`Run ${mes.data.ClusterType} Command: ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/${mes.data.Cmd} ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/${mes.data.Cmd}`, JSON.stringify(mes.data.Payload));
                    break;
                case "run-cluster-group-command":
                    console.log(`Run Cluster Group Command: ucl/by-unid/${mes.data.Unid}/Groups/Commands/${mes.data.Cmd} ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/Groups/Commands/${mes.data.Cmd}`, JSON.stringify(mes.data.Payload));
                    break;
                case "update-cluster-attr":
                    console.log(`Update ${mes.data.ClusterType} Attribute: ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/WriteAttributes ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/WriteAttributes`, JSON.stringify(mes.data.Payload));
                    break;
                case "update-metric-interval":
                    console.log(`Update System Metric Reporting Interval: ucl/by-machine-id/${mes.data.Mid}/SystemMetrics/Commands/WriteAttributes ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-machine-id/${mes.data.Mid}/SystemMetrics/Commands/WriteAttributes`, JSON.stringify(mes.data.Payload));
                    break;
                case "force-read-cluster-attr":
                    console.log(`Force Read ${mes.data.ClusterType} Attributes: ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/ForceReadAttributes ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/ForceReadAttributes`, JSON.stringify(mes.data.Payload));
                    break;
                case "run-group-command":
                    console.log(`Run Group Command: ucl/by-group/${mes.data.GroupId}/${mes.data.ClusterType}/Commands/${mes.data.Cmd} ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-group/${mes.data.GroupId}/${mes.data.ClusterType}/Commands/${mes.data.Cmd}`, JSON.stringify(mes.data.Payload));
                    break;
                case "clear-mqtt":
                    console.log(`Clear retained messages by ${mes.data.type}`);
                    switch (mes.data.type) {
                        case "by-unid":
                            handler.clearMqttByUnid();
                            break;
                        case "by-group":
                            handler.clearMqttByGroup();
                            break;
                        case "OTA":
                            handler.clearMqttOTA();
                            break;
                    }
                    mqttConnection.end();
                    setTimeout(() => connect(mqttConnectionProps), 700);
                    break;
                case "toggle-upti":
                    console.log(`Toggle PTI ${mes.data.SerialNumber}: ucl/UPTICap/${mes.data.SerialNumber}/Toggle ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/UPTICap/${mes.data.SerialNumber}/Toggle`, JSON.stringify(mes.data.Payload));
                    if (mes.data.Payload.Enable) {
                        handler.state.UPTI.Trace[mes.data.SerialNumber] = [];
                        handler.addToQueue("upti-trace", {
                            SerialNumber: mes.data.SerialNumber,
                            Trace: []
                        });
                    }
                    break;
                case "discovery-upti":
                    console.log(`Discovery UPTI: ucl/UPTICap/Discovery`);
                    mqttConnection.publish(`ucl/UPTICap/Discovery`, '{}');
                    break;
            }
        } catch {
            console.log(`Could not process command: '${JSON.stringify(message)}'`);
        }
    });

    connection.on('close', function(connection) {
        console.log(`${new Date()}: Peer '${connection.remoteAddress}' disconnected from WebSocket`);
        handler.clients.splice(index, 1);
    });
}

function connect(data) {
    console.log(`${data.Host}:${data.Port}, TLS: ${data.TLS}`);
    if (mqttConnection && mqttConnection.connected == true) {
        handler.sendToAll(handler.getInitState(mqttConnectionProps));
    } else {
        handler.resetState();
        // Connect client to MQTT broker
        let options = {
            port: data.Port,
            host: data.Host,
            keepalive: 10000
        };
        if (data.TLS) {
            if (!handler.state.Certificates || !handler.state.Certificates.CA || !handler.state.Certificates.ClientKey || !handler.state.Certificates.ClientCertificate) {
                handler.sendToAll(handler.getErrorResponse("Not all certificates are listed"));
                handler.sendToAll({ type: "connect", data: false });
                return;
            }
            options.ca = handler.state.Certificates.CA.Data;
            options.cert = handler.state.Certificates.ClientCertificate.Data;
            options.key = handler.state.Certificates.ClientKey.Data;
            options.rejectUnauthorized = true;
            options.protocol = 'mqtts';
        }
        let counter = 50;
        mqttConnection = mqtt.connect(options);
        var waitConnect = setInterval(() => {
            if (mqttConnection && mqttConnection.connected) {
                mqttConnection.on("message", onMqttMessage);
                mqttConnection.on("close", disconnect);
                mqttConnectionProps = data;
                topics.forEach(topic => { mqttConnection.subscribe(topic) });
                clearInterval(waitConnect);
                handler.sendToAll({ type: "connect", data: true });
            } else if (counter-- <= 0) {
                mqttConnection = {};
                clearInterval(waitConnect);
                handler.sendToAll(handler.getErrorResponse("Connection to MQTT Failed"));
                handler.sendToAll({ type: "connect", data: false });
            }
        }, 100);
    }
}

function disconnect() {
    console.log("Disconnect from MQTT");
    handler.resetState();
    handler.sendToAll({ type: "connect", data: false });
}

function onMqttMessage(topic, message) {
    //Depending on the topic will determine the way to save the inputs on messages
    let response = {};
    if (topic.match(/ucl\/by-unid\/(.*)\/(ep\d+)\/OTA\/(.*)/)) {
        response = handler.processOTAByUnid(topic, message);
    } else if (topic.match(/ucl\/by-unid\/(.*)\/(.*)\/Groups/)) {
        response = handler.processClusterGroup(topic, message);
        handler.addToQueue("nodes-list", handler.state.Nodes);
    } else if (topic.match(/ucl\/by-unid\/(.*)\/(ep\d+)\/Location\/Attributes\/(.*)\/(.*)\/Reported/)) {
        response = processLocator(topic, message);
        handler.addToQueue("nodes-list", handler.state.Nodes);
    } else if (topic.match(/ucl\/by-unid\/(.*)\/(ep\d+)\/(.*)/)) {
        response = handler.processCluster(topic, message);
        handler.addToQueue("nodes-list", handler.state.Nodes);
    } else if (topic.match(/ucl\/by-unid\/.*\/ProtocolController\/NetworkManagement/)) {
        response = handler.processProtocolController(topic, message);
        handler.addToQueue("nodes-list", handler.state.Nodes);
    } else if (topic.match(/ucl\/by-unid\/(.*)\/ProtocolController\/RFTelemetry\/(.*)/)) {
        response = handler.processRFTelemetry(topic, message);
    } else if (topic.match(/ucl\/by-unid\/.*\/State/)) {
        response = handler.processNodeState(topic, message);
        handler.addToQueue("nodes-list", handler.state.Nodes);
    } else if (topic.match(/ucl\/by-machine-id\/(.*)\/SystemMetrics\/(.*)/)) {
        response = handler.processSystemMetrics(topic, message);
    } else if (topic.match(/ucl\/UPTICap\/.*\/TracePackage/)) {
        response = handler.processUPTITrace(topic, message);
    } else if (topic.match(/ucl\/by-group\/.*/)) {
        response = handler.processGroup(topic, message);
    } else if (topic.match(/ucl\/OTA\/info\/(.*)\/(.*)/)) {
        response = handler.processOTAInfo(topic, message);
    } else if (topic === "ucl/SmartStart/List") {
        response = handler.processSmartStart(topic, message);
    } else if (topic === "ucl/UPTICap/List") {
        response = handler.processUPTIList(topic, message);
    } else if (topic.match(/ucl\/by-mqtt-client\/(.*)\/ApplicationMonitoring\/Attributes\/(.*)/)) {
        response = handler.processAppMonitoringList(topic, message);
    } else if (topic.match(/ucl\/SmartStart\/CommissionableDevice\/(.*)/)) {
        response = handler.processCommissionableDevices(topic, message);
    }
    if (response && Object.keys(response).length > 0)
        handler.addToQueue(response.type, response.data);
}

function saveFile(file) {
    handler.state.Certificates[file.CertName] = { Name: file.fileName, Data: file.data };
    let certs = getCertificates();
    console.log("certificates: " + JSON.stringify(certs));
    handler.sendToAll({ type: "certificates", data: certs });
    console.log(file.fileName + " uploaded successfully!");
}

function getCertificates() {
    let cert = {};
    Object.keys(handler.state.Certificates).forEach(key => {
        cert[key] = handler.state.Certificates[key].Name;
    });
    return cert;
}