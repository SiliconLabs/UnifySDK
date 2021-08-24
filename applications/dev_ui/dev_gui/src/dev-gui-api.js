"use strict";

let webSocketServer = require('websocket').server;
let http = require('http');
let mqtt = require('mqtt');
var fs = require('fs');
let path = require('path');
let supportedClusters = require('./cluster-types/supported-clusters.js').SupportedClusters;

process.title = 'dev-gui-server';
const webSocketsServerPort = 1337;

let mqttConnection;
let mqttConnectionProps = {
    Host: "",
    Port: 0,
    UseTLS: false
};

let clients = [];
let state = {
    Nodes: [],
    SmartStart: [],
    Groups: [],
    Certificates: {},
    OTA: []
};

let topics = ["ucl/SmartStart/List",
    "ucl/by-unid/+/State",
    "ucl/by-unid/+/State/SupportedCommands",
    "ucl/by-unid/+/ProtocolController/NetworkManagement",
    "ucl/by-unid/+/+/Groups/#",
    `ucl/by-unid/+/+/OTA/#`,
    "ucl/OTA/info/+/+",
];
Object.keys(supportedClusters).forEach((i) => {
    topics.push(`ucl/by-unid/+/+/${i}/SupportedCommands`);
    topics.push(`ucl/by-unid/+/+/${i}/Attributes/#`);
});
console.log(`TopicList:${JSON.stringify(topics)}`);

let server = http.createServer(function(request, response) {});
server.listen(webSocketsServerPort, function() {
    console.log(`${new Date()}: Server is listening on port ${webSocketsServerPort}`);
});

let wsServer = new webSocketServer({ httpServer: server });

wsServer.on('request', function(request) {
    console.log("Certificates:" + JSON.stringify(getCertificates()));
    let connection = request.accept(undefined, request.origin);
    let index = clients.push(connection) - 1;
    console.log(`${new Date()}: Connection WebSocket accepted`);
    if (mqttConnection && mqttConnection.connected == true)
        sendToAll(getInitState());
    sendToAll({ type: "certificates", data: getCertificates() });

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
                    console.log("Retrieve Smart Start List");
                    let response = {
                        type: "smart-start-list",
                        data: state.SmartStart
                    };
                    sendToAll(response);
                    break;
                case "run-smart-start-command":
                    console.log(`Run Smart Start Command: ucl/SmartStart/List/${mes.data.Cmd} ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/SmartStart/List/${mes.data.Cmd}`, JSON.stringify(mes.data.Payload));
                    break;
                case "run-node-command":
                    console.log(`Run Node Command: ucl/by-unid/${mes.data.Unid}/ProtocolController/NetworkManagement/Write ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/ProtocolController/NetworkManagement/Write`, JSON.stringify(mes.data.Payload));
                    break;
                case "run-state-command":
                    console.log(`Run State Command: ucl/by-unid/${mes.data.Unid}/State/Commands/${mes.data.Payload}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/State/Commands/${mes.data.Payload}`);
                    break;
                case "run-cluster-command":
                    console.log(`Run ${mes.data.ClusterType} Command: ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/${mes.data.Cmd} ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/${mes.data.Cmd}`, JSON.stringify(mes.data.Payload));
                    break;
                case "run-cluster-group-command":
                    console.log(`Run Cluster Group Command: ucl/by-unid/${mes.data.Unid}/Groups/Commands/${mes.data.Cmd} ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/Groups/Commands/${mes.data.Cmd}`, JSON.stringify(mes.data.Payload));
                    break;
                case "run-cluster-scene-command":
                    console.log(`Run Cluster Scene Command: ucl/by-unid/${mes.data.Unid}/Cluster/Scenes/${mes.data.Cmd} ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/Cluster/Scenes/${mes.data.Cmd}`, JSON.stringify(mes.data.Payload));
                    break;
                case "update-cluster-attr":
                    console.log(`Update ${mes.data.ClusterType} Attribute: ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/WriteAttributes ${JSON.stringify(mes.data.Payload)}`);
                    mqttConnection.publish(`ucl/by-unid/${mes.data.Unid}/${mes.data.ClusterType}/Commands/WriteAttributes`, JSON.stringify(mes.data.Payload));
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
                            clearMqttByUnid();
                            break;
                        case "by-group":
                            clearMqttByGroup();
                            break;
                        case "OTA":
                            clearMqttOTA();
                            break;
                    }
                    mqttConnection.end();
                    setTimeout(() => connect(mqttConnectionProps), 700);
            }
        } catch {
            console.log(`Could not process command: '${JSON.stringify(message)}'`);
        }
    });

    connection.on('close', function(connection) {
        console.log(`${new Date()}: Peer '${connection.remoteAddress}' disconnected from WebSocket`);
        clients.splice(index, 1);
    });
});

function sendToAll(response) {
    if (!response || Object.keys(response).length === 0) return;
    let json = JSON.stringify(response);
    for (let i = 0; i < clients.length; i++) {
        clients[i].sendUTF(json);
    }
}

function connect(data) {
    console.log(`${data.Host}:${data.Port}, TLS: ${data.TLS}`);
    if (mqttConnection && mqttConnection.connected == true) {
        sendToAll(getInitState());
    } else {
        resetState();
        // Connect client to MQTT broker
        let options = {
            port: data.Port,
            host: data.Host,
            keepalive: 10000
        };
        if (data.TLS) {
            if (!state.Certificates || !state.Certificates.CA || !state.Certificates.ClientKey || !state.Certificates.ClientCertificate) {
                sendToAll(getErrorResponse("Not all certificates are listed"));
                sendToAll({ type: "connect", data: false });
                return;
            }
            options.ca = state.Certificates.CA.Data;
            options.cert = state.Certificates.ClientCertificate.Data;
            options.key = state.Certificates.ClientKey.Data;
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
                sendToAll({ type: "connect", data: true });
            } else if (counter-- <= 0) {
                mqttConnection = {};
                clearInterval(waitConnect);
                sendToAll(getErrorResponse("Connection to MQTT Failed"));
                sendToAll({ type: "connect", data: false });
            }
        }, 100);
    }
}

function disconnect() {
    console.log("Disconnect from MQTT");
    resetState();
    sendToAll({ type: "connect", data: false });
}

function getInitState() {
    return {
        type: "init-state",
        data: {
            Port: mqttConnectionProps.Port,
            Host: mqttConnectionProps.Host,
            IsConnected: true,
            Nodes: state.Nodes,
            SmartStart: state.SmartStart,
            Groups: state.Groups,
            OTA: state.OTA
        }
    };
}

function onMqttMessage(topic, message) {
    //Depending on the topic will determine the way to save the inputs on messages
    let response = {};
    if (topic == "ucl/SmartStart/List") {
        response = processSmartStart(topic, message);
    } else if (topic.match(/ucl\/by-unid\/.*\/ProtocolController\/NetworkManagement/)) {
        response = processProtocolController(topic, message);
    } else if (topic.match(/ucl\/by-unid\/.*\/State/)) {
        response = processNodeState(topic, message);
    } else if (topic.match(/ucl\/by-unid\/(.*)\/(ep\d+)\/OTA\/(.*)/)) {
        response = processOTAByUnid(topic, message);
    } else if (topic.match(/ucl\/by-unid\/(.*)\/(.*)\/Groups/)) {
        response = processGroup(topic, message);
    } else if (topic.match(/ucl\/by-unid\/(.*)\/(ep\d+)\/(.*)/)) {
        response = processCluster(topic, message);
    } else if (topic.match(/ucl\/OTA\/info\/(.*)\/(.*)/)) {
        response = processOTAInfo(topic, message);
    }

    sendToAll(response);
    response = {
        type: "nodes-list",
        data: state.Nodes
    };
    sendToAll(response);
}

function initEp(node, ep) {
    if (!node.ep)
        node.ep = {};
    if (!node.ep[ep])
        node.ep[ep] = { Clusters: {}, OTA: {} };
    return node.ep[ep];
}

function initProp(prop, path, payload) {
    let propName = path.shift();
    if (!prop[propName])
        prop[propName] = {};
    if (path.length)
        initProp(prop[propName], path, payload);
    else
        prop[propName] = payload;
}

function getNodeByUnid(unid) {
    let index = state.Nodes.findIndex(i => i.Unid == unid);
    if (index === -1)
        index = state.Nodes.push({ Unid: unid, ClusterTypes: [], OTA: {} }) - 1;

    return state.Nodes[index];
}

function getErrorResponse(message) {
    console.log(message);
    return {
        type: "error",
        data: message
    }
}

function resetState() {
    state.SmartStart = [];
    state.Nodes = [];
    state.Groups = [];
    state.OTA = [];
}

function saveFile(file) {
    state.Certificates[file.CertName] = { Name: file.fileName, Data: file.data };
    let certs = getCertificates();
    console.log("certificates: " + JSON.stringify(certs));
    sendToAll({ type: "certificates", data: certs });
    console.log(file.fileName + " uploaded successfully!");
}

function getCertificates() {
    let cert = {};
    Object.keys(state.Certificates).forEach(key => {
        cert[key] = state.Certificates[key].Name;
    });
    return cert;
}

function clearMqttByUnid() {
    state.Nodes.forEach(node => {
        if (node.ep)
            Object.keys(node.ep).forEach(epName => {
                if (node.ep[epName].Groups) {
                    if (node.ep[epName].Groups.SupportedCommands)
                        mqttConnection.publish(`ucl/by-unid/${node.Unid}/${epName}/Groups/SupportedCommands`, "");
                    if (node.ep[epName].Groups.Attributes)
                        Object.keys(node.ep[epName].Groups.Attributes).forEach(attr => removeAttr(node.Unid, epName, "Groups", node.ep[epName].Groups.Attributes[attr], attr));
                }
                if (node.ep[epName].OTA && node.ep[epName].OTA.Attributes)
                    Object.keys(node.ep[epName].OTA.Attributes).forEach(uiid => {
                        removeAttr(node.Unid, epName, cluster, node.OTA[uiid].Attributes[uiid], uiid);
                    });
                if (node.ep[epName].Clusters)
                    Object.keys(node.ep[epName].Clusters).forEach(cluster => {
                        if (node.ep[epName].Clusters[cluster].Attributes)
                            Object.keys(node.ep[epName].Clusters[cluster].Attributes).forEach(attr => {
                                removeAttr(node.Unid, epName, cluster, node.ep[epName].Clusters[cluster].Attributes[attr], attr);
                            });
                        if (cluster.SupportedCommands)
                            mqttConnection.publish(`ucl/by-unid/${node.Unid}/${epName}/${cluster}/SupportedCommands`, "");
                    });
            });
        if (node.SupportedCommands)
            mqttConnection.publish(`ucl/by-unid/${node.Unid}/State/SupportedCommands`, "");
        if (node.NetworkStatus)
            mqttConnection.publish(`ucl/by-unid/${node.Unid}/State`, "");
        if (node.ClusterTypes.indexOf("ProtocolController") > -1)
            mqttConnection.publish(`ucl/by-unid/${node.Unid}/ProtocolController/NetworkManagement`, "");
    });
}

function clearMqttByGroup() {
    state.Groups.forEach(group => {
        mqttConnection.publish(`ucl/by-group/${group.GroupId}/GroupName`, "");
        if (group.NodeList)
            group.NodeList.forEach(unid => mqttConnection.publish(`ucl/by-group/${group.GroupId}/NodeList/${unid}`, ""));
        if (group.Clusters)
            group.Clusters.forEach(cluster => mqttConnection.publish(`ucl/by-group/${group.GroupId}/${cluster}/SupportedCommands`, ""));
    });
}

function clearMqttOTA() {
    state.OTA.forEach(i => {
        if (i.Unid && i.Unid.length > 0)
            i.Unid.forEach(j => mqttConnection.publish(`ucl/OTA/info/${i.Uiid}/${j}`, ""));
        else
            mqttConnection.publish(`ucl/OTA/info/${i.Uiid}/all`, "");
    });
}

function removeAttr(unid, ep, cluster, attr, attrName) {
    if (!attr.Desired && !attr.Reported)
        Object.keys(attr).forEach(i => {
            removeAttr(unid, ep, cluster, attr[i], `${attrName}/${i}`);
        });
    if (attr.Desired !== null && attr.Desired !== undefined)
        mqttConnection.publish(`ucl/by-unid/${unid}/${ep}/${cluster}/Attributes/${attrName}/Desired`, "");
    if (attr.Reported !== null && attr.Reported !== undefined)
        mqttConnection.publish(`ucl/by-unid/${unid}/${ep}/${cluster}/Attributes/${attrName}/Reported`, "");
}

function processSmartStart(topic, message) {
    console.log(`Received Smart Start List: '${message}'`);
    if (message.toString() == "")
        return;
    try {
        let list = JSON.parse(message);
        state.SmartStart = list && list.value;
        return {
            type: "smart-start-list",
            data: state.SmartStart
        }
    } catch (error) {
        return getErrorResponse(`Failed Parsing Smart Start List: ${error}`);
    }
}

function processProtocolController(topic, message) {
    let match = topic.match(/ucl\/by-unid\/(.*)\/ProtocolController\/.*/);

    if (match === null || message.toString() == "" || match.length < 2)
        return;

    try {
        let unid = match[1];
        console.log(`Received ProtocolController: unid='${unid}', mes='${message}'`);
        let node = getNodeByUnid(unid);
        let data = JSON.parse(message.toString());
        node.State = data.State;
        if (node.ClusterTypes.indexOf("ProtocolController") === -1)
            node.ClusterTypes.push("ProtocolController");
        data.Unid = unid;
        return {
            type: "protocol-controller-state",
            data: data
        };
    } catch (err) {
        return getErrorResponse("Failed Parsing State of Controller.");
    }
}

function processNodeState(topic, message) {
    let index = -1;
    let match = topic.match(/ucl\/by-unid\/(.*)\/State\/*(.*)/);

    if (match === null)
        return;
    try {
        let unid = match[1];
        console.log(`Received State: unid='${unid}', topic='${topic}', mes='${message}'`);
        index = state.Nodes.findIndex(i => i.Unid === unid);
        if (message.toString() == "" && (match[2] === "" || index === -1)) {
            if (index != -1) {
                console.log(`Removing Node Entry: '${unid}'`);
                state.Nodes.splice(index, 1);
            }
        } else {
            let node = getNodeByUnid(unid);
            let payload = message.toString() == "" ? null : JSON.parse(message);
            if (match.length > 2 && match[2])
                initProp(node, match[2].split('/'), payload && payload.value);
            else
                for (let key in payload) {
                    node[key] = payload[key];
                }
                // used to check that inclusion process completed
            return {
                type: "state",
                data: node
            };
        }
    } catch (err) {
        return getErrorResponse("Failed parsing State: " + err);
    }
}

function processGroup(topic, message) {
    let match = topic.match(/ucl\/by-unid\/(.*)\/(.*)\/Groups\/(.*)/);
    if (!match || match.length < 4)
        return;

    try {
        let unid = match[1];
        console.log(`Received Cluster Group: unid='${unid}', topic='${topic}', mes='${message}'`);
        if (message.toString() === "" && state.Nodes.findIndex(i => i.Unid == unid) == -1)
            return;
        let node = getNodeByUnid(unid);
        let ep = initEp(node, match[2]);
        let path = match[3].split('/');
        if (!path.length)
            return;
        let payload = message.toString() === "" ? null : JSON.parse(message);
        if (!ep.Groups)
            ep.Groups = {};
        initProp(ep.Groups, path, payload && payload.value);
    } catch (err) {
        return getErrorResponse(`Failed Parsing Cluster Group: ${err}`);
    }
}

function processCluster(topic, message) {
    let index = -1;
    let match = topic.match(/ucl\/by-unid\/(.*)\/(ep\d+)\/(.*)/)
    if (match === null || match.length < 4) {
        console.log(`Unknown topic: '${topic}'; data: '${message}'`);
        return;
    }

    let unid = match[1];
    let ep = match[2];
    let path = match[3].split('/');
    let clusterType = path.shift();

    try {
        console.log(`Received Cluster ${clusterType}: unid='${unid}/${ep}', topic='${topic}', mes='${message}'`);
        index = state.Nodes.findIndex(i => i.Unid === unid);
        if (message.toString() === "" && index === -1)
            return;
        let node = getNodeByUnid(unid);
        let endpoint = initEp(node, ep);
        if (!endpoint.Clusters[clusterType])
            endpoint.Clusters[clusterType] = {};

        if (node.ClusterTypes.indexOf(clusterType) === -1)
            node.ClusterTypes.push(clusterType);
        let payload = message.toString() === "" ? null : JSON.parse(message);
        initProp(endpoint.Clusters[clusterType], path, payload && payload.value);
    } catch (error) {
        return getErrorResponse(`Failed Parsing Cluster ${clusterType}`);
    }
}

function processOTAByUnid(topic, message) {
    let match = topic.match(/ucl\/by-unid\/(.*)\/(ep\d+)\/OTA\/(.*)/);
    try {
        let unid = match[1];
        let ep = match[2];
        let path = match[3].replace(/[/]$/, "").split('/');
        let uiid = path.shift();

        if (!match || match.length < 4 || ep === "" || unid === "")
            return;
        let index = state.Nodes.findIndex(i => i.Unid === unid);
        if (message.toString() === "" && index === -1)
            return;
        console.log(`Received OTA info: unid='${unid}', uiid='${uiid}', topic='${topic}', mes='${message}'`);

        let node = getNodeByUnid(unid);
        let endpoint = initEp(node, ep);
        let payload = message.toString() === "" ? null : JSON.parse(message);
        if (path.length === 0 && payload === null) {
            console.log(`Removing OTA Entry: '${uiid}'`);
            endpoint.OTA[uiid] = {};
        } else {
            if (!endpoint.OTA[uiid])
                endpoint.OTA[uiid] = {};
            initProp(endpoint.OTA[uiid], path, payload && payload.value);
        }
    } catch (err) {
        return getErrorResponse("Cluster OTA info error: " + err);
    }

}

function processOTAInfo(topic, message) {
    let index = -1;
    let match = topic.match(/ucl\/OTA\/info\/(.*)\/(.*)/)
    if (match === null || match.length < 3 || match[1] === "" || match[2] === "") {
        console.log(`Unknown topic: '${topic}'; data: '${message}'`);
        return;
    }
    let uiid = match[1];
    let unid = match[2];
    try {
        console.log(`Received OTA info: uiid='${uiid}' unid='${unid}', topic='${topic}', mes='${message}'`);
        index = state.OTA.findIndex(i => i.Uiid === uiid);
        if (message.toString() === "") {
            if (index === -1) return;
            if (unid === "all") {
                console.log(`Removing OTA Image Entry: '${uiid}'`);
                state.OTA.splice(index, 1);
            } else {
                let unidIndex = state.OTA[index].Unid.indexOf(unid);
                if (unidIndex === -1) return;
                console.log(`Removing OTA Image UNID Entry: '${unid}'`);
                state.OTA[index].Unid.splice(unidIndex, 1);
                if (!state.OTA[index].Unid.length) {
                    console.log(`Removing OTA Image Entry: '${uiid}'`);
                    state.OTA.splice(index, 1);
                }
            }
        } else {
            let image = JSON.parse(message);
            if (index === -1) {
                image.Uiid = uiid;
                image.Unid = (unid === "" || unid === "all") ? [] : [unid];
                state.OTA.push(image) - 1;
            } else {
                Object.keys(image).forEach(key => {
                    state.OTA[index][key] = image[key];
                });
                if (unid === "all")
                    state.OTA[index].Unid = [];
                else if (unid !== "" && state.OTA[index].Unid.indexOf(unid) === -1)
                    state.OTA[index].Unid.push(unid);
            }
        }
        return {
            type: "ota",
            data: state.OTA
        };
    } catch (error) {
        return getErrorResponse(`Failed Parsing OTA`);
    }
}