import * as React from 'react';
import { Button, Modal } from 'react-bootstrap';
import QrReader from 'react-qr-reader';
import { QrCodeDlgProps, QrCodeDlgState } from './qr-code-dlg-types';
import sha1 from 'sha1';

class QrCodeDlg extends React.Component<QrCodeDlgProps, QrCodeDlgState> {
    constructor(props: QrCodeDlgProps) {
        super(props);
        this.state = {
            ShowModal: false,
            Error: "",
            ItemIndex: 0
        };
    }

    updateState(show: boolean, itemIndex: number) {
        this.setState({ ShowModal: show, ItemIndex: itemIndex });
    }

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    handleScan = (data: any) => {
        this.setState({
            Error: ""
        })
        if (data) {
            if (isNaN(data)) {
                this.setState({ Error: "Incorrect Qr-Code format" });
                return;
            }
            try {
                let sha = sha1(data.slice(9));
                let shaArray: number[] = [];
                for (let index = 0; index < sha.length; index = index + 2) {
                    shaArray.push(parseInt("0x" + sha.slice(index, index + 2)));
                }
                let sha1CheckSum = (shaArray[0] << 8) + shaArray[1];
                let dataCheckSum = parseInt(data.slice(4, 4 + 5))
                if (sha1CheckSum !== dataCheckSum) {
                    this.setState({ Error: "Checksum is incorrect" });
                    return;
                }

                let dsk = data.slice(12, 12 + 40);
                this.props.UpdateDSK(this.state.ItemIndex, "DSK", dsk);
                this.toggleModal(false);
            } catch (error) {
                this.setState({ Error: "Can't parse DSK" });
                return;
            }
        }
    }

    handleError = (err: any) => {
        this.setState({
            Error: err.message
        })
    }

    render() {
        return (
            <Modal size="sm" show={this.state.ShowModal} onHide={() => this.toggleModal(false)} >
                <Modal.Header>
                    <Modal.Title>Scan QR-code</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    <QrReader
                        delay={300}
                        onError={this.handleError}
                        onScan={this.handleScan}
                        style={{ width: '265px', transform: 'scaleX(-1)' }}
                    />
                    <div className="error-modal-header row padding-h-15" hidden={this.state.Error.length === 0}>{this.state.Error}</div>
                </Modal.Body>
                <Modal.Footer>
                    <Button variant="primary" onClick={() => this.toggleModal(false)}>Cancel</Button>
                </Modal.Footer>
            </Modal>
        );
    }
}

export default QrCodeDlg