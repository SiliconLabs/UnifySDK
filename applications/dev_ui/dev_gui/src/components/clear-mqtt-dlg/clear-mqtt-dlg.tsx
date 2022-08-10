import { MenuItem, TextField } from '@mui/material';
import * as React from 'react';
import { Button, Modal } from 'react-bootstrap';
import { ClearMqttDlgProps, ClearMqttDlgState } from './clear-mqtt-dlg-types';

class ClearMqttDlg extends React.Component<ClearMqttDlgProps, ClearMqttDlgState> {
    constructor(props: ClearMqttDlgProps) {
        super(props);
        this.state = {
            ShowModal: false,
            Type: "by-unid"
        };
    }

    typeList = ["by-unid", "by-group", "OTA"];

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    clearMqtt = () => {
        this.props.SocketServer.send(JSON.stringify({
            type: "clear-mqtt",
            data: {
                type: this.state.Type
            }
        }));
        this.toggleModal(false);
    }

    render() {
        return (
            <Modal show={this.state.ShowModal} onHide={() => this.toggleModal(false)}>
                <Modal.Header onMouseDown={(e: any) => e?.stopPropagation()} >
                    <Modal.Title>
                        Clear Mqtt Messages
                    </Modal.Title>
                </Modal.Header>
                <Modal.Body onMouseDown={(e: any) => e?.stopPropagation()}>
                    <TextField size="small" fullWidth={true} select label="Message Type"
                        value={this.state.Type} defaultValue={this.state.Type}
                        onChange={(e) => this.setState({ Type: e.target.value })} variant="outlined">
                        {this.typeList.map((option: any) => {
                            return <MenuItem key={option} value={option}>
                                {option}
                            </MenuItem>
                        })}
                    </TextField>
                </Modal.Body>
                <Modal.Footer onMouseDown={(e: any) => e?.stopPropagation()}>
                    <Button variant="outline-primary" onClick={this.clearMqtt}>OK</Button>
                    <Button variant="primary" onClick={() => this.toggleModal(false)}>Cancel</Button>
                </Modal.Footer>
            </Modal >
        );
    }
}

export default ClearMqttDlg