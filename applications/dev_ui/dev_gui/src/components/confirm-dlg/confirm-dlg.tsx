import * as React from 'react';
import { Button, Modal } from 'react-bootstrap';
import { ConfirmDlgProps, ConfirmDlgState } from './confirm-dlg-types';


class ConfirmDlg extends React.Component<ConfirmDlgProps, ConfirmDlgState> {
    constructor(props: ConfirmDlgProps) {
        super(props);
        this.state = {
            ShowModal: false,
            Title: "",
            Body: ""
        };
    }

    update(title: string, body: string) {
        this.setState({ ShowModal: true, Title: title, Body: body });
    }

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    render() {
        return (
            <Modal show={this.state.ShowModal} onHide={() => this.toggleModal(false)} >
                <Modal.Header>
                    <Modal.Title>
                        {this.state.Title}
                    </Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    {this.state.Body}
                </Modal.Body>
                <Modal.Footer>
                    <Button variant="outline-primary" onClick={() => { this.props.ConfirmAction(); this.toggleModal(false); }}>Yes</Button>
                    <Button variant="primary" onClick={() => this.toggleModal(false)}>No</Button>
                </Modal.Footer>
            </Modal>
        );
    }
}

export default ConfirmDlg