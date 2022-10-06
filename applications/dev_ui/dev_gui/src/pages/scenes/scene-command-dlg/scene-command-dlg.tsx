import { MenuItem, TextField } from '@mui/material';
import * as React from 'react';
import { Button, Col, Form, Modal, Row } from 'react-bootstrap';
import { Link } from 'react-router-dom';
import { ClusterTypeAttrs } from '../../../cluster-types/cluster-type-attributes';
import { ClusterTypes } from '../../../cluster-types/cluster-types';
import CommandAttrs from '../../../components/command-atts/command-attrs';
import { SceneCommandDlgProps, SceneCommandDlgState } from './scene-command-dlg-types';

class SceneCommandDlg extends React.Component<SceneCommandDlgProps, SceneCommandDlgState> {
    constructor(props: SceneCommandDlgProps) {
        super(props);
        this.state = {
            ClusterType: "",
            Command: {},
            AvailableCommands: [],
            ShowModal: false
        };
        this.changeCommandAttrs = React.createRef();
    }
    changeCommandAttrs: any;

    toggleModal = (value: boolean) => {
        this.setState({ ShowModal: value });
    }

    updateState(clusterType: string, command: string, payload: any) {
        clusterType = clusterType || ClusterTypes.OnOff;
        let commands = ClusterTypeAttrs[clusterType].server.commands || [];
        let currentCommand = command ? commands.find((i: any) => i.name === command) : commands[0];
        this.setState({
            ClusterType: clusterType,
            ShowModal: true,
            AvailableCommands: commands,
            Command: currentCommand
        }, () => this.changeCommandAttrs.current.updateState(this.state.Command, payload));
    }

    handleClusterChange = (event: any) => {
        let commands = ClusterTypeAttrs[event.target.value].server.commands || [];
        this.setState({ ClusterType: event.target.value, AvailableCommands: commands, Command: commands[0] },
            () => this.changeCommandAttrs.current.updateState(this.state.Command));
    }

    handleCommandChange = (event: any) => {
        this.setState({ Command: this.state.AvailableCommands.find((i: any) => i.id === Number(event.target.value)) },
            () => this.changeCommandAttrs.current.updateState(this.state.Command));
    }

    saveCommand = () => {
        if (!this.state.Command)
            return;
        if (!this.props.Scene.SceneTableExtensions)
            this.props.Scene.SceneTableExtensions = {};
        if (!this.props.Scene.SceneTableExtensions[this.state.ClusterType])
            this.props.Scene.SceneTableExtensions[this.state.ClusterType] = {};
        this.props.Scene.SceneTableExtensions[this.state.ClusterType][this.state.Command.name] = this.changeCommandAttrs.current.state.Payload;
        this.props.SocketServer.send(JSON.stringify(
            {
                type: "save-scene",
                data: {
                    GroupID: this.props.Scene.GroupID,
                    SceneID: this.props.Scene.SceneID,
                    Payload: this.props.Scene
                }
            }));
        this.toggleModal(false);
    }

    render() {
        return (
            <Modal show={this.state.ShowModal} size="lg" onHide={() => this.toggleModal(false)}>
                <Modal.Header closeButton>
                    <Modal.Title>Scene Command</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    <div className="row">
                        <div className="col-sm-6 inline margin-v-10">
                            <TextField size="small" className="flex-input" fullWidth={true} select label="Cluster Type" name="ClusterType"
                                value={this.state.ClusterType} onChange={this.handleClusterChange} variant="outlined">
                                {Object.keys(ClusterTypeAttrs)?.map((cluster: any, index: number) => {
                                    return <MenuItem key={index} value={cluster}>
                                        {cluster}
                                    </MenuItem>
                                })}
                            </TextField>
                        </div>
                        <div className="col-sm-6 inline margin-v-10">
                            <TextField size="small" className="flex-input" fullWidth={true} select label="Command" name="Command" disabled={!this.state.ClusterType || !this.state.Command}
                                value={this.state.Command && this.state.Command.id} onChange={this.handleCommandChange} variant="outlined">
                                {ClusterTypeAttrs[this.state.ClusterType]?.server.commands?.map((cmd: any, ind: number) => {
                                    return <MenuItem key={ind} value={cmd.id}>
                                        {cmd.name}
                                    </MenuItem>
                                })}
                            </TextField>
                        </div>
                    </div>
                    <hr />
                    <Row>
                        <Col xs={12} className="text-center" hidden={this.state.Command && this.state.Command.fields?.length > 0}>
                            <span className="no-content">No Attributes</span>
                        </Col>
                    </Row>
                    <CommandAttrs ref={this.changeCommandAttrs} />
                </Modal.Body>
                <Modal.Footer>
                    <Link to={`/scenes/${this.props.Scene.GroupID}/${this.props.Scene.SceneID}`} onClick={() => { this.saveCommand(); }}>
                        <Button variant="primary" disabled={!this.state.Command} >
                            Save
                        </Button>
                    </Link>
                    <Button variant="outline-primary" onClick={() => this.toggleModal(false)}>
                        Cancel
                    </Button>
                </Modal.Footer>
            </Modal>
        );
    }
}

export default SceneCommandDlg