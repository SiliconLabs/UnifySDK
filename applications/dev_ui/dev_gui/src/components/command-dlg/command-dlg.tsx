import * as React from 'react';
import { Button, Modal } from 'react-bootstrap';
import CommandAttrs from '../command-atts/command-attrs';
import { CommandDlgProps, CommandDlgState } from './command-dlg-types';


class CommandDlg extends React.Component<CommandDlgProps, CommandDlgState> {
    constructor(props: CommandDlgProps) {
        super(props);
        this.sendCommand = this.sendCommand.bind(this);
        this.toggleModal = this.toggleModal.bind(this);
        this.state = {
            Command: {},
            Unid: "",
            ShowModal: false
        };
        this.changeCommandAttrs = React.createRef();
    }
    changeCommandAttrs: any;

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    updateState(unid: string, command: any, showModal: boolean) {
        this.setState({ Unid: unid, Command: command, ShowModal: showModal }, () => this.changeCommandAttrs.current.updateState(this.state.Command));
    }

    sendCommand() {
        this.props.SocketServer.send(JSON.stringify(
            {
                type: "run-cluster-command",
                data: {
                    Unid: this.state.Unid,
                    ClusterType: this.props.ClusterType,
                    Cmd: this.state.Command.name,
                    Payload: this.changeCommandAttrs.current.state.Payload
                }
            }));
    }

    render() {
        return (
            <Modal show={this.state.ShowModal} size="lg" onHide={() => this.toggleModal(false)}>
                <Modal.Header closeButton>
                    <Modal.Title>Fill Command Attribute(s)</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    <CommandAttrs ref={this.changeCommandAttrs} />
                </Modal.Body>
                <Modal.Footer>
                    <Button variant="primary" onClick={() => { this.sendCommand(); this.toggleModal(false); }}>
                        Send
                    </Button>
                    <Button variant="outline-primary" onClick={() => this.toggleModal(false)}>
                        Cancel
                    </Button>
                </Modal.Footer>
            </Modal>
        );
    }
}

export default CommandDlg