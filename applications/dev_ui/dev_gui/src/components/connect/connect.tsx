import * as React from 'react';
import './connect.css';
import Button from 'react-bootstrap/Button';
import { ConnectionState, ConnectionProps } from './connect-types';
import { Form } from 'react-bootstrap';
import { Tooltip } from '@material-ui/core';
import ClearMqttDlg from '../clear-mqtt-dlg/clear-mqtt-dlg';

class Connect extends React.Component<ConnectionProps, ConnectionState> {
    constructor(props: ConnectionProps) {
        super(props);
        this.storage = this.props.Storage.Get();
        this.state = {
            Host: this.storage.Host || 'localhost',
            Port: this.storage.Port || 1883,
            Status: "No connection established",
            IsError: false,
            TLS: this.storage.TLS || false,
            CA: null,
            CAName: this.storage.CAName || "",
            ClientKey: null,
            ClientKeyName: this.storage.ClientKeyName || "",
            ClientCertificate: null,
            ClientCertificateName: this.storage.ClientCertificateName || ""
        }
        this.fileRefs = {
            uploadCA: React.createRef(),
            uploadCK: React.createRef(),
            uploadCC: React.createRef()
        };
        this.changeClearMqttDlg = React.createRef();
    }
    fileRefs: any;
    storage: any;
    changeClearMqttDlg: any;

    componentDidUpdate(prevProps: ConnectionProps) {
        if (prevProps !== this.props) {
            this.setState({
                Status: this.props.IsConnected === true ? "Connected" : (this.props.IsConnected === false ? "Disconnected" : "Connecting..."),
                Host: this.state.Host
            });
        }
    }

    handleChange = (event: any) => {
        this.updateAttr(event.target.name, event.target.value);
    };

    handleCheckboxChange = (event: any) => {
        this.updateAttr(event.target.name, event.target.checked);
    };

    handleFileChange = (event: any) => {
        this.uploadFile(event.target.files[0], event.target.name);
        this.updateAttr(event.target.name, event.target.files[0], event.target.files[0].name);
    };

    updateAttr(name: string, value: any, storageValue?: any) {
        let prop: any = {};
        prop[name] = value;
        if (storageValue !== undefined) {
            prop[name + "Name"] = storageValue;
            this.props.Storage.Set(name + "Name", storageValue);
        } else {
            this.props.Storage.Set(name, value);
        }
        this.setState(prop);
    }

    browseFile = (event: any) => {
        let name: any = `upload${event?.target.name}`;
        this.fileRefs[name].current?.click();
    }

    connect = (event: any) => {
        event.stopPropagation();
        if (this.props.IsConnected === true) {
            this.props.SocketServer.send(JSON.stringify({ type: "disconnect", data: null }));
        }
        else {
            if (!this.state.Host || !this.state.Port || (this.state.TLS && (!this.state.CAName || !this.state.ClientKeyName || !this.state.ClientCertificateName))) {
                this.setState({ IsError: true });
                this.props.handleIsConnectedChange(false);
                return;
            }
            this.setState({ IsError: false });
            var data = { Host: this.state.Host, Port: this.state.Port, TLS: this.state.TLS };
            this.props.handleIsConnectedChange(null);
            this.props.SocketServer.send(JSON.stringify({ type: "connect", data: data }));
        }
    }

    uploadFile(file: any, certName: string) {
        let reader = new FileReader();
        reader.onload = (e) => {
            this.props.SocketServer.send(JSON.stringify({ type: "fileUpload", fileType: "certificate", CertName: certName, fileName: file.name, data: e?.target?.result }));
        }
        reader.readAsBinaryString(file);
    }

    clearMqtt = (e: any) => {
        e.stopPropagation();
        this.changeClearMqttDlg.current?.toggleModal(true)
    }

    render() {
        return (<>
            {this.props.IsCollapsed
                ? <div className="col-sm-9 float-left margin-v-10">
                    <div className="offset-sm-1 margin-v-10 col-sm-3  float-left">
                        Host: <b>{this.state.Host}:{this.state.Port}</b>
                    </div>
                    <div className="col-sm-3 float-left">
                        <Form.Label column>
                            <div className="check-container">
                                <Form.Check checked={this.state.TLS} readOnly={true} />
                            </div>
                            <span className="padding-l-5">TLS</span>
                        </Form.Label>
                    </div>
                    <div className="col-sm-3 margin-v-10 float-left">
                        Status: <b>{this.state.Status}</b>
                    </div>
                    {this.props.IsConnected === true
                        ? <Button variant="outline-primary" className="float-right" type="button" onMouseDown={this.connect}>Disconnect</Button>
                        : <Button variant="primary" className="float-right" disabled={!this.state.Host || !this.state.Port || this.props.IsConnected === null} type="button" onMouseDown={this.connect}>Connect</Button>}
                </div>
                : <div className="col-sm-10 float-left">
                    <h5 className="margin-b-0">UIC MQTT broker connection</h5>
                    <div className="con-container col-sm-12">
                        <div className="col-sm-12 flex">
                            <Form.Label column className="col-sm-5p"></Form.Label>
                            <div className="con-item col-sm-30p">
                                <div className="item-title col-sm-4">Host</div>
                                <div className="item-value">
                                    <input type="text" name="Host" className={`${this.state.IsError && !this.state.Host ? "error-border" : ""}`} disabled={this.props.IsConnected !== false} value={this.state.Host} onMouseDown={(e) => e?.stopPropagation()} onChange={this.handleChange} />
                                </div>
                            </div>
                            <div className="con-item col-sm-30p">
                                <div className="item-title col-sm-4">Port</div>
                                <div className="item-value">
                                    <input type="text" name="Port" pattern="#" className={`${this.state.IsError && !this.state.Port ? "error-border" : ""}`} disabled={this.props.IsConnected !== false} value={this.state.Port} onMouseDown={(e) => e?.stopPropagation()} onChange={this.handleChange} />
                                </div>
                            </div>
                            <div className="con-item col-sm-30p">
                                <div className="item-title col-sm-4">Status</div>
                                <div className="item-value">
                                    <b>{this.state.Status}</b>
                                </div>
                            </div>
                        </div>
                        <div className="col-sm-12 flex">
                            <Form.Label column className="col-sm-5p" onMouseDown={(e: any) => e?.stopPropagation()} >
                                <div className="check-container">
                                    <Form.Check checked={this.state.TLS} name="TLS" disabled={this.props.IsConnected !== false} onChange={this.handleCheckboxChange} />
                                </div>
                                <span className="padding-l-5">TLS</span>
                            </Form.Label>
                            <div className="con-item col-sm-30p">
                                <div className="item-title col-sm-4">CA</div>
                                <div className="item-value">
                                    <input className="hidden" type="file" name="CA" ref={this.fileRefs.uploadCA} onChange={this.handleFileChange} />
                                    <Tooltip title="Click to change CA file" disableHoverListener={!this.state.TLS} disableTouchListener={!this.state.TLS}  >
                                        <input type="text" name="CA" className={`${this.state.IsError && !this.state.CAName ? "error-border" : ""} pointer`} disabled={!this.state.TLS || this.props.IsConnected !== false} readOnly defaultValue={this.state.CAName} placeholder="CA File" onMouseDown={(e) => e?.stopPropagation()} onClick={this.browseFile} />
                                    </Tooltip>
                                </div>
                            </div>
                            <div className="con-item col-sm-30p">
                                <div className="item-title col-sm-4">Key</div>
                                <div className="item-value">
                                    <input className="hidden" type="file" name="ClientKey" ref={this.fileRefs.uploadCK} onChange={this.handleFileChange} />
                                    <Tooltip title="Click to change ClientKey file" disableHoverListener={!this.state.TLS} disableTouchListener={!this.state.TLS}  >
                                        <input type="text" name="CK" className={`${this.state.IsError && !this.state.ClientKeyName ? "error-border" : ""} pointer`} disabled={!this.state.TLS || this.props.IsConnected !== false} readOnly defaultValue={this.state.ClientKeyName} placeholder="ClientKey File" onMouseDown={(e) => e?.stopPropagation()} onClick={this.browseFile} />
                                    </Tooltip>
                                </div>
                            </div>
                            <div className="con-item col-sm-30p">
                                <div className="item-title col-sm-4">Certificate</div>
                                <div className="item-value">
                                    <input className="hidden" type="file" name="ClientCertificate" ref={this.fileRefs.uploadCC} onChange={this.handleFileChange} />
                                    <Tooltip title="Click to change ClientCertificate file" disableHoverListener={!this.state.TLS} disableTouchListener={!this.state.TLS}  >
                                        <input type="text" name="CC" className={`${this.state.IsError && !this.state.ClientCertificateName ? "error-border" : ""} pointer`} disabled={!this.state.TLS || this.props.IsConnected !== false} readOnly defaultValue={this.state.ClientCertificateName} placeholder="ClientCertificate File" onMouseDown={(e) => e?.stopPropagation()} onClick={this.browseFile} />
                                    </Tooltip>
                                </div>
                            </div>
                        </div>
                        <div className="col-sm-12">
                            <div className="connect-btn">
                                {this.props.IsConnected === true
                                    ? <>
                                        <Button className="margin-r-5" variant="outline-primary" type="button" onMouseDown={this.clearMqtt}>Clear Mqtt Messages</Button>
                                        <Button variant="outline-primary" type="button" onMouseDown={this.connect}>Disconnect</Button>
                                    </>
                                    : <Button variant="primary" disabled={!this.state.Host || !this.state.Port || this.props.IsConnected === null} type="button" onMouseDown={this.connect}>Connect</Button>}
                            </div>
                        </div>
                    </div>
                </div >}
            <ClearMqttDlg SocketServer={this.props.SocketServer} ref={this.changeClearMqttDlg} />
        </>
        )
    }
}
export default Connect