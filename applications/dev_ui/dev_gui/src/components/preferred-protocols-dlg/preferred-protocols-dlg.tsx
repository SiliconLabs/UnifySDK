import * as React from 'react';
import { Button, Form, ListGroup, Modal } from 'react-bootstrap';
import * as IoIcons from 'react-icons/io';
import Tooltip from '@material-ui/core/Tooltip';
import { PreferredProtocolsDlgProps, PreferredProtocolsDlgState, Protocol } from './preferred-protocols-dlg-types';
import './preferred-protocols.css'


class PreferredProtocolsDlg extends React.Component<PreferredProtocolsDlgProps, PreferredProtocolsDlgState> {
    constructor(props: PreferredProtocolsDlgProps) {
        super(props);
        this.toggleModal = this.toggleModal.bind(this);
        this.state = {
            SmartStartItem: {},
            PreferredProtocols: [],
            ShowModal: false
        };
    }

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    save() {
        let smartStart = this.state.SmartStartItem;
        smartStart.PreferredProtocols = [];
        this.state.PreferredProtocols.forEach((i, index) => {
            if (this.state.PreferredProtocols[index].Enabled)
                smartStart.PreferredProtocols.push(this.state.PreferredProtocols[index].Name)
        });
        this.props.SaveFunc(smartStart);
    }

    updateState(item: any, showModal: boolean) {
        let protocols: Protocol[] = [{
            Name: "Z-Wave",
            Enabled: false
        },
        {
            Name: "Z-Wave Long Range",
            Enabled: false
        }];
        if (item.PreferredProtocols && item.PreferredProtocols.length > 0) {
            if (item.PreferredProtocols[0] !== protocols[0].Name)
                protocols.reverse();
            protocols[0].Enabled = true;
            protocols[1].Enabled = item.PreferredProtocols.length > 1;
        }

        this.setState({ SmartStartItem: item, ShowModal: showModal, PreferredProtocols: protocols });
    }

    handleChange(index: number, event: any) {
        let protocols = this.state.PreferredProtocols;
        protocols[index].Enabled = event.target.checked;
        this.setState({ PreferredProtocols: protocols });
    }

    reverse() {
        this.setState({ PreferredProtocols: this.state.PreferredProtocols.reverse() });
    }

    render() {
        return (
            <Modal show={this.state.ShowModal} size="sm" onHide={() => this.toggleModal(false)}>
                <Modal.Header closeButton>
                    <Modal.Title>Preferred Protocols</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    {(this.state.PreferredProtocols && this.state.PreferredProtocols.length === 2) ?
                        <ListGroup className="preferred-protocols">
                            <ListGroup.Item>
                                <Form.Check className="float-left" checked={this.state.PreferredProtocols[0].Enabled} onChange={this.handleChange.bind(this, 0)} />
                                <span className="float-left">{this.state.PreferredProtocols[0].Name}</span>
                                <Tooltip className="float-right" title="Move Down"><span className="icon"><IoIcons.IoMdArrowRoundDown onClick={()=>this.reverse()} /></span></Tooltip>
                            </ListGroup.Item>
                            <ListGroup.Item>
                                <Form.Check className="float-left" checked={this.state.PreferredProtocols[1].Enabled} onChange={this.handleChange.bind(this, 1)} />
                                <span className="float-left">{this.state.PreferredProtocols[1].Name}</span>
                                <Tooltip className="float-right" title="Move Up"><span className="icon"><IoIcons.IoMdArrowRoundUp onClick={() => this.reverse()} /></span></Tooltip>
                            </ListGroup.Item>
                        </ListGroup>
                        : <></>}
                </Modal.Body >
                <Modal.Footer>
                    <Button variant="primary" onClick={() => { this.save(); this.toggleModal(false); }}>
                        Save
                    </Button>
                    <Button variant="outline-primary" onClick={() => this.toggleModal(false)}>
                        Cancel
                    </Button>
                </Modal.Footer>
            </Modal >
        );
    }
}

export default PreferredProtocolsDlg