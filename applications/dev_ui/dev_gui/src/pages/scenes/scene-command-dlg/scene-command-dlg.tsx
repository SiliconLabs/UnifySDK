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
                        <div className="col-md-6">
                            <Form.Label column>Cluster Type</Form.Label>
                            <div>
                                <Form.Control as="select" name="ClusterType" onChange={this.handleClusterChange} value={this.state.ClusterType}>
                                    {Object.keys(ClusterTypeAttrs)?.map((cluster: any, index: number) => {
                                        return <option key={index} value={cluster}>{cluster}</option>
                                    })}
                                </Form.Control>
                            </div>
                        </div>
                        <div className="col-md-6">
                            <Form.Label column>Command</Form.Label>
                            <div>
                                <Form.Control as="select" name="Command" onChange={this.handleCommandChange} value={this.state.Command && this.state.Command.id} disabled={!this.state.ClusterType || !this.state.Command}>
                                    {ClusterTypeAttrs[this.state.ClusterType]?.server.commands?.map((cmd: any, ind: number) => {
                                        return <option key={ind} value={cmd.id}>{cmd.name}</option>
                                    })}
                                </Form.Control>
                            </div>
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