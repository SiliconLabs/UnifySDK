import * as React from 'react';
import { Button, Col, Form, Modal, Row } from 'react-bootstrap';
import { ToastContainer, toast } from 'react-toastify';
import { ClusterTypeAttrs } from '../../cluster-types/cluster-type-attributes';
import { Group } from '../../pages/groups/groups-types';
import CommandAttrs from '../command-atts/command-attrs';
import { GroupCommandDlgProps, GroupCommandDlgState } from './group-command-dlg-types';


class GroupCommandDlg extends React.Component<GroupCommandDlgProps, GroupCommandDlgState> {
    constructor(props: GroupCommandDlgProps) {
        super(props);
        this.sendCommand = this.sendCommand.bind(this);
        this.toggleModal = this.toggleModal.bind(this);
        this.state = {
            ClusterType: "",
            Command: {},
            Group: {} as Group,
            AvailableClusters: [],
            AvailableCommands: [],
            ShowModal: false
        };
        this.changeCommandAttrs = React.createRef();
    }
    changeCommandAttrs: any;

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    updateState(group: Group, showModal: boolean) {
        if (!group.Clusters.length) {
            toast(`This group does not contain any commands`, { type: "warning" })
            return;
        }
        let cluster = group.Clusters[0];
        let advertisedCommands = (group as any)[cluster].SupportedCommands
        let commands = ClusterTypeAttrs[cluster].server.commands.filter((i: any) => advertisedCommands.indexOf(i.name) > -1);
        this.setState({
            Group: group,
            ClusterType: cluster,
            ShowModal: showModal,
            AvailableCommands: commands,
            Command: commands[0]
        }, () => this.changeCommandAttrs.current.updateState(this.state.Command));
    }

    handleClusterChange = (event: any) => {
        let advertisedCommands = (this.state.Group as any)[event.target.value].SupportedCommands;
        let commands = ClusterTypeAttrs[event.target.value].server.commands.filter((i: any) => advertisedCommands.indexOf(i.name) > -1);
        this.setState({ ClusterType: event.target.value, AvailableCommands: commands, Command: commands[0] },
            () => this.changeCommandAttrs.current.updateState(this.state.Command));
    }

    handleCommandChange = (event: any) => {
        this.setState({ Command: this.state.AvailableCommands.find((i: any) => i.id === Number(event.target.value)) },
            () => this.changeCommandAttrs.current.updateState(this.state.Command));
    }

    sendCommand() {
        if (!this.state.Command)
            return;

        this.props.SocketServer.send(JSON.stringify(
            {
                type: "run-group-command",
                data: {
                    GroupId: this.state.Group.GroupId,
                    ClusterType: this.state.ClusterType,
                    Cmd: this.state.Command.name,
                    Payload: this.changeCommandAttrs.current.state.Payload
                }
            }));
    }

    render() {
        return (
            <Modal show={this.state.ShowModal} size="lg" onHide={() => this.toggleModal(false)}>
                <Modal.Header closeButton>
                    <Modal.Title>Send Group Command</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    <div className="row">
                        <div className="col-md-6">
                            <Form.Label column>Cluster Type</Form.Label>
                            <div>
                                <Form.Control as="select" name="ClusterType" onChange={this.handleClusterChange} value={this.state.ClusterType}>
                                    {this.state.Group.Clusters?.map((cluster: any, index: number) => {
                                        return <option key={index} value={cluster}>{cluster}</option>
                                    })}
                                </Form.Control>
                            </div>
                        </div>
                        <div className="col-md-6">
                            <Form.Label column>Command</Form.Label>
                            <div>
                                <Form.Control as="select" name="Command" onChange={this.handleCommandChange} value={this.state.Command && this.state.Command.id} disabled={!this.state.ClusterType || !this.state.Command}>
                                    {this.state.AvailableCommands.map((cmd: any, ind: number) => {
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
                    <Button variant="primary" disabled={!this.state.Command} onClick={() => { this.sendCommand(); }}>
                        Send
                    </Button>
                    <Button variant="primary" disabled={!this.state.Command} onClick={() => { this.sendCommand(); this.toggleModal(false); }}>
                        Send&Close
                    </Button>
                    <Button variant="outline-primary" onClick={() => this.toggleModal(false)}>
                        Cancel
                    </Button>
                </Modal.Footer>
                <ToastContainer
                    position="bottom-right"
                    autoClose={4000}
                    hideProgressBar={false}
                    newestOnTop={false}
                    closeOnClick
                    rtl={false}
                    pauseOnFocusLoss
                    draggable
                    pauseOnHover />
            </Modal>
        );
    }
}

export default GroupCommandDlg