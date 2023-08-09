let clients = [];
let state = {
    Nodes: [],
    SmartStart: [],
    Groups: [],
    Certificates: {},
    OTA: [],
    UPTI: { List: [], Trace: {} },
    SystemMetrics: {},
    AppMonitoringList: {},
    CommissionableDevices: [],
    updateNodeList: true
};
let queue = new Map();
let measurementClusterList = ["CarbonDioxide", "CarbonMonoxide", "RelativityHumidity", "SoilMoisture", "PM25"];

async function runQueueHandler() {
    while (true) {
        queue.forEach((value, key, map) => {
            map.delete(key);
            sendToAll({ type: key, data: value });
        });
        await new Promise(resolve => setTimeout(resolve, 500));
    }
}

exports.processSmartStart = (topic, message) => {
    console.log(`Received SmartStart List: '${message}'`);
    if (message.toString() == "")
        return;
    try {
        let list = JSON.parse(message);
        state.SmartStart = list && list.value;
        return {
            type: "smart-start-list",
            data: state.SmartStart
        };
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processCluster = (topic, message) => {
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
        let payload = message.toString() === "" ? null : JSON.parse(message);
        payload = payload && payload.value !== undefined ? payload.value : payload;
        if (clusterType.endsWith("Measurement") || measurementClusterList.indexOf(clusterType) > -1) {
            if (!endpoint.Measurements[clusterType])
                endpoint.Measurements[clusterType] = {};
            initProp(endpoint.Measurements[clusterType], path, payload);
        } else {
            if (!endpoint.Clusters[clusterType])
                endpoint.Clusters[clusterType] = {};
            if (node.ClusterTypes.indexOf(clusterType) === -1)
                node.ClusterTypes.push(clusterType);
            initProp(endpoint.Clusters[clusterType], path, payload);
        }
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processSystemMetrics = (topic, message) => {
    let match = topic.match(/ucl\/by-machine-id\/(.*)\/SystemMetrics\/(.*)/)
    if (match === null || match.length < 3) {
        console.log(`Unknown topic: '${topic}'; data: '${message}'`);
        return;
    }

    let mid = match[1];
    let path = match[2].split('/');

    try {
        console.log(`Received Cluster SystemMetrics: machine-id='${mid}', topic='${topic}', mes='${message}'`);
        if (!state.SystemMetrics[mid])
            state.SystemMetrics[mid] = {};

        let payload = message.toString() === "" ? null : JSON.parse(message);
        payload = payload && payload.value !== undefined ? payload.value : payload;
        if (path[0] === "Attributes")
            path.pop();
        initProp(state.SystemMetrics[mid], path, payload);
        return {
            type: "system-metrics",
            data: state.SystemMetrics
        };
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processOTAByUnid = (topic, message) => {
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
        addToQueue("nodes-list", state.Nodes);
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processOTAInfo = (topic, message) => {
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
        addToQueue("ota", state.OTA);
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processClusterGroup = (topic, message) => {
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
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processGroup = (topic, message) => {
    let match = topic.match(/ucl\/by-group\/(.*)/);
    if (!match || match.length < 2)
        return;
    try {
        let path = match[1].split('/');
        let groupId = parseInt(path.shift());
        console.log(`Received Group: unid='${groupId}', topic='${topic}', mes='${message}'`);
        let group = state.Groups.find(i => i.GroupId === groupId);
        if (message.toString() === "" && !group)
            return;

        if (!group) {
            group = { GroupId: groupId, NodeList: {} };
            state.Groups.push(group);
        }

        let payload = message.toString() === "" ? null : JSON.parse(message);
        initProp(group, path, payload && payload.value);
        state.Groups.forEach((i, index) => {
            if (i.NodeList === undefined)
                state.Groups.splice(index, 1);
        });
        return {
            type: "groups",
            data: state.Groups
        }
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processLocator = (topic, message) => {
    let index = -1;
    let match = topic.match(/ucl\/by-unid\/(.*)\/(ep\d+)\/Location\/Attributes\/(.*)\/(.*)\/Reported/)
    if (match === null || match.length < 5) {
        console.log(`Unknown topic: '${topic}'; data: '${message}'`);
        return;
    }

    state.OTA = [];
    state.UPTI = { List: [], Trace: {} };
    let unid = match[1];
    let ep = match[2];
    let tag = match[3];
    let attr = match[4];
    let clusterType = attr === "Position" ? "MultiLocator" : "Locator";

    try {
        console.log(`Received Cluster ${clusterType}: unid='${unid}/${ep}', tag='${tag}', topic='${topic}', mes='${message}'`);
        index = state.Nodes.findIndex(i => i.Unid === unid);
        if (message.toString() === "" && index === -1)
            return;
        let node = getNodeByUnid(unid);
        let endpoint = initEp(node, ep);
        if (!endpoint.Clusters[clusterType])
            endpoint.Clusters[clusterType] = {};
        if (!endpoint.Clusters[clusterType][attr])
            endpoint.Clusters[clusterType][attr] = {};
        if (!endpoint.Clusters[clusterType][attr][tag])
            endpoint.Clusters[clusterType][attr][tag] = {};
        if (node.ClusterTypes.indexOf(clusterType) === -1)
            node.ClusterTypes.push(clusterType);
        let payload = message.toString() === "" ? null : JSON.parse(message);
        endpoint.Clusters[clusterType][attr][tag] = payload;
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processProtocolController = (topic, message) => {
    let match = topic.match(/ucl\/by-unid\/(.*)\/ProtocolController\/.*/);

    if (match === null || message.toString() == "" || match.length < 2)
        return;

    try {
        let unid = match[1];
        console.log(`Received ProtocolController: unid='${unid}', mes='${message}'`);
        let node = getNodeByUnid(unid);
        let data = JSON.parse(message.toString());
        node.NetworkManagementState = data;
        if (node.ClusterTypes.indexOf("ProtocolController") === -1)
            node.ClusterTypes.push("ProtocolController");
        data.Unid = unid;
        return {
            type: "protocol-controller-state",
            data: data
        };
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processUPTIList = (topic, message) => {
    try {
        console.log(`Received UPTI List: topic='${topic}', mes='${message}'`);
        if (message.toString() === "") {
            state.UPTI.Trace = {};
            state.UPTI.List = [];
        } else {
            state.UPTI.List = JSON.parse(message).value;
        }

        return {
            type: "upti-list",
            data: state.UPTI.List
        }
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processUPTITrace = (topic, message) => {
    let match = topic.match(/ucl\/UPTICap\/(.*)\/TracePackage/);
    if (!match || match.length < 2)
        return;
    try {
        let serial = match[1];
        console.log(`Received UPTI Trace: serial='${serial}', topic='${topic}', mes='${message}'`);
        let pti = state.UPTI.List.find(i => i.SerialNumber === serial);
        let payload = message.toString() === "" ? null : JSON.parse(message);
        if (!pti || !payload)
            return;

        if (!state.UPTI.Trace[serial])
            state.UPTI.Trace[serial] = [];

        let trace = state.UPTI.Trace[serial];
        if (trace.length > 200)
            trace.shift();

        trace.push(payload);

        return {
            type: "upti-trace",
            data: {
                SerialNumber: serial,
                Trace: payload
            }
        }
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processNodeState = (topic, message) => {
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
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processRFTelemetry = (topic, message) => {
    let index = -1;
    let match = topic.match(/ucl\/by-unid\/(.*)\/ProtocolController\/RFTelemetry\/(.*)/);
    if (match === null || match.length < 3) {
        console.log(`Unknown topic: '${topic}'; data: '${message}'`);
        return;
    }

    let unid = match[1];
    let path = match[2].split('/');
    if (path[0] === "Commands")
        return;
    try {
        console.log(`Received RFTelemetry: unid='${unid}', topic='${topic}', mes='${message}'`);
        index = state.Nodes.findIndex(i => i.Unid === unid);
        if (message.toString() === "" && index === -1)
            return;
        let node = getNodeByUnid(unid);
        if (!node.RFTelemetry)
            node.RFTelemetry = {
                TxReport: {
                    Statistics: {
                        Count: 0,
                        SuccessRate: null,
                        AverageTime: 0,
                        AverageRepeaters: 0
                    },
                    Destinations: {}
                }
            };
        let payload = message.toString() === "" ? null : JSON.parse(message);
        if (path[path.length - 1] === "TxReport") {
            if (!node.RFTelemetry.TxReport.Destinations[payload.DestinationUNID])
                node.RFTelemetry.TxReport.Destinations[payload.DestinationUNID] = {
                    Statistics: {
                        Count: 0,
                        SuccessRate: null,
                        AverageTime: 0,
                        AverageRepeaters: 0
                    },
                    Report: {}
                };

            node.RFTelemetry.TxReport.Destinations[payload.DestinationUNID].Report = payload;
            setRFTelemetryStatistics(node.RFTelemetry.TxReport.Statistics, payload);
            setRFTelemetryStatistics(node.RFTelemetry.TxReport.Destinations[payload.DestinationUNID].Statistics, payload);
        } else {
            initProp(node.RFTelemetry, path, payload && payload.value);
        }
        addToQueue("nodes-list", state.Nodes);
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processAppMonitoringList = (topic, message) => {
    let match = topic.match(/ucl\/by-mqtt-client\/(.*)\/ApplicationMonitoring\/Attributes\/(.*)/);
    if (match === null || match.length < 3) {
        console.log(`Unknown topic: '${topic}'; data: '${message}'`);
        return;
    }

    let client = match[1];
    let path = match[2].split('/');
    try {
        console.log(`Received ApplicationMonitoring: cliend='${client}', topic='${topic}', mes='${message}'`);
        if (message.toString() === "")
            return;
        if (!state.AppMonitoringList[client])
            state.AppMonitoringList[client] = {};

        let payload = message.toString() === "" ? null : JSON.parse(message);
        initProp(state.AppMonitoringList[client], path, payload && payload.value);
        addToQueue("app-monitoring", state.AppMonitoringList);
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.processCommissionableDevices = (topic, message) => {
    let match = topic.match(/ucl\/SmartStart\/CommissionableDevice\/(.*)/);
    if (message.toString() == "")
        return;
    if (match === null || match.length < 2) {
        console.log(`Unknown topic: '${topic}'; data: '${message}'`);
        return;
    }
    try {
        console.log(`Received Commissionable Device: topic='${topic}', mes='${message}'`);
        let identifier = match[1];
        let payload = JSON.parse(message);
        let item = state.CommissionableDevices.find(i => i.Id === identifier);
        if (item) {
            item.QRCode = payload.QRCode;
            item.DSK = payload.DSK;
        } else {
            state.CommissionableDevices.push({ Id: identifier, QRCode: payload.QRCode, DSK: payload.DSK });
        }
        return {
            type: "commissionable-device",
            data: state.CommissionableDevices
        };
    } catch (error) {
        return getErrorResponse(topic, message, error);
    }
}

exports.clearMqttByUnid = () => {
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

exports.clearMqttByGroup = () => {
    state.Groups.forEach(group => {
        Object.keys(group).forEach(key => {
            if (key !== "GroupId")
                switch (key) {
                    case "NodeList":
                        Object.keys(group[key]).forEach(unid => mqttConnection.publish(`ucl/by-group/${group.GroupId}/NodeList/${unid}`, ""));
                        break;
                    case "GroupName":
                        mqttConnection.publish(`ucl/by-group/${group.GroupId}/GroupName`, "");
                        break;
                    default:
                        mqttConnection.publish(`ucl/by-group/${group.GroupId}/${key}/SupportedCommands`, "", { retain: true });
                        break;
                }
        });
    });
}

exports.clearMqttOTA = () => {
    state.OTA.forEach(i => {
        if (i.Unid && i.Unid.length > 0)
            i.Unid.forEach(j => mqttConnection.publish(`ucl/OTA/info/${i.Uiid}/${j}`, ""));
        else
            mqttConnection.publish(`ucl/OTA/info/${i.Uiid}/all`, "");
    });
}

exports.resetState = () => {
    state.SmartStart = [];
    state.Nodes = [];
    state.Groups = [];
    state.OTA = [];
    state.UPTI = { List: [], Trace: {} };
    state.CommissionableDevices = [];
}

exports.getInitState = (mqttConnectionProps) => {
    return {
        type: "init-state",
        data: {
            Port: mqttConnectionProps.Port,
            Host: mqttConnectionProps.Host,
            IsConnected: true,
            Nodes: state.Nodes,
            SmartStart: state.SmartStart,
            Groups: state.Groups,
            OTA: state.OTA,
            UPTI: state.UPTI,
            SystemMetrics: state.SystemMetrics,
            AppMonitoringList: state.AppMonitoringList,
            CommissionableDevices: state.CommissionableDevices
        }
    };
}

exports.setUpdateNodeList = (value) => {
    state.updateNodeList = value;
}

function addToQueue(type, data) {
    queue.set(type, data);
}

function sendToAll(response) {
    if (!response || Object.keys(response).length === 0) return;
    let json = JSON.stringify(response);
    for (let i = 0; i < clients.length; i++) {
        clients[i].sendUTF(json);
    }
}

function getErrorResponse(topic, message = null, error = null) {
    let data = message === null || error === null ? [topic] : ["Failed Parsing Topic:", `${topic} ${message}`, `Error: ${error}`];
    console.log(JSON.stringify(error));
    return {
        type: "error",
        data: data
    }
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

function setRFTelemetryStatistics(statistics, payload) {
    let count = statistics.Count++;
    let successTx = statistics.SuccessRate === null ? 0 : Math.round(count * statistics.SuccessRate);
    statistics.SuccessRate = (payload.TransmissionSuccessful ? ++successTx : successTx) / statistics.Count;
    statistics.AverageTime = ((statistics.AverageTime * count) + payload.TransmissionTimeMs) / statistics.Count;
    statistics.AverageRepeaters = ((statistics.AverageRepeaters * count) + payload.LastRouteRepeaters.length) / statistics.Count;
}

function initEp(node, ep) {
    if (!node.ep)
        node.ep = {};
    if (!node.ep[ep])
        node.ep[ep] = { Clusters: {}, OTA: {}, Measurements: {} };
    return node.ep[ep];
}

function initProp(prop, path, payload) {
    let propName = path.shift();
    if (path.length) {
        if (!prop[propName])
            prop[propName] = {};
        initProp(prop[propName], path, payload);
        if (Object.keys(prop[propName]).length === 0) {
            delete(prop[propName]);
        }
    } else {
        if (payload === null && prop[propName] !== undefined) {
            delete(prop[propName])
        } else {
            if (!prop[propName])
                prop[propName] = {};
            prop[propName] = payload;
        }
    }
}

function getNodeByUnid(unid) {
    let index = state.Nodes.findIndex(i => i.Unid == unid);
    if (index === -1)
        index = state.Nodes.push({
            Unid: unid,
            ClusterTypes: [],
            OTA: {}
        }) - 1;

    return state.Nodes[index];
}

exports.sendToAll = sendToAll;
exports.addToQueue = addToQueue;
exports.runQueueHandler = runQueueHandler;
exports.getErrorResponse = getErrorResponse;
exports.clients = clients;
exports.state = state;