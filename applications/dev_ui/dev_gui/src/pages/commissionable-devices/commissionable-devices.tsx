import React from 'react';
import { Button, Card, Col, Form, Modal, Row, Table } from 'react-bootstrap';
import * as FiIcons from 'react-icons/fi';
import { TextField, Tooltip } from '@mui/material';
import { CommissionableDevicesProps, CommissionableDevicesState } from './commissionable-devices-types';
import QRCode from "react-qr-code";
import { CommissionableDevice } from '../../app-types';

export class CommissionableDevices extends React.Component<CommissionableDevicesProps, CommissionableDevicesState> {
  constructor(props: CommissionableDevicesProps) {
    super(props);
    this.state = {
      ShowModal: false,
      ProcessedItem: {} as CommissionableDevice
    };
  }

  add = (dsk: any) => {
    this.setState({
      ProcessedItem: {
        DSK: dsk,
        Unid: "",
        Include: true,
        ProtocolControllerUnid: ""
      },
      ShowModal: true
    });
  }

  addToList = () => {
    this.props.SocketServer.send(JSON.stringify({ type: "run-smart-start-command", data: { Cmd: "Update", Payload: this.state.ProcessedItem } }));
  }

  toggleModal = (value: boolean) => {
    this.setState({ ShowModal: value });
  }

  handleChange = (isCheckBox: boolean, event: any) => {
    let item = this.state.ProcessedItem;
    item[event.target.name] = isCheckBox ? event.target.checked : event.target.value;
    this.setState({ ProcessedItem: item });
  }

  render() {
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    return (
      <>
        <h3>Commisionable Devices</h3>
        {(!this.props.List || !this.props.List.length || !this.props.IsConnected)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : (isMobile
            ? <div className='table-content smart-start'>
              {
                this.props.List.map((item, index) => {
                  return (
                    <Card key={index} className="inline margin-v-10">
                      <Card.Header className='flex'>
                        <div className="col-sm-10">
                          Identifier: <b>{item.Id}</b>
                        </div>
                        <div className="col-sm-2 float-right" hidden={!item.DSK} >
                          <Tooltip title="Add to SmartStart List" className="float-right">
                            <span className="icon">
                              <FiIcons.FiPlus color="#28a745" className="margin-h-5" onClick={() => this.add(item.DSK)} />
                            </span>
                          </Tooltip>
                        </div>
                      </Card.Header>
                      <Card.Body>
                        <div className='col-sm-12'>
                          <div className="col-sm-12 inline padding-v-5" hidden={!item.DSK}><b><i>DSK: </i></b>{item.DSK}</div>
                          <div className="col-sm-12 inline padding-v-5"><b><i>QR-Code: </i></b>{item.QRCode}</div>
                          <div className="col-sm-12 inline padding-v-5"><QRCode value={item.QRCode} className="col-sm-12" /></div>
                        </div>
                      </Card.Body>
                    </Card>
                  );
                })}
            </div>
            : <Table striped hover>
              <thead>
                <tr className="">
                  <th>Identifier</th>
                  <th className="text-center">QR-Code</th>
                  <th>DSK</th>
                  <th></th>
                </tr>
              </thead>
              <tbody>
                {this.props.List.map((item, index) =>
                  <tr key={index}>
                    <td className="vertical-middle">{item.Id}</td>
                    <td className="vertical-middle text-center"><div><QRCode value={item.QRCode} className="col-sm-12" />{item.QRCode}</div></td>
                    <td className="vertical-middle">{item.DSK}</td>
                    <td className="vertical-middle"><Button hidden={!item.DSK} variant="primary" onClick={() => this.add(item.DSK)}>Add to SmartStart List</Button></td>
                  </tr>
                )}
              </tbody>
            </Table>
          )}
        <Modal show={this.state.ShowModal} size="lg" onHide={() => this.toggleModal(false)}>
          <Modal.Header closeButton>
            <Modal.Title>Add '{this.state.ProcessedItem.DSK}' to SmartStart List</Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <div className="col-sm-12">
              <Form.Label column sm="11">
                <div className="check-container">
                  <Form.Check name="Include" checked={this.state.ProcessedItem.Include} onChange={this.handleChange.bind(this, true)} />
                </div>
                <span className='word-break'>Include</span></Form.Label>
            </div>
            <div className="col-sm-12 margin-v-10">
              <TextField size="small" className="flex-input" fullWidth={true} label="Unid" variant="outlined" type="text" name="Unid" value={this.state.ProcessedItem.Unid} onChange={this.handleChange.bind(this, false)} />
            </div>
            <div className="col-sm-12 margin-v-10">
              <TextField size="small" className="flex-input" fullWidth={true} label="ProtocolControllerUnid" variant="outlined" type="text" name="ProtocolControllerUnid" value={this.state.ProcessedItem.ProtocolControllerUnid} onChange={this.handleChange.bind(this, false)} />
            </div>
          </Modal.Body>
          <Modal.Footer>
            <Button variant="primary" onClick={() => { this.addToList(); this.toggleModal(false); }}>
              Add
            </Button>
            <Button variant="outline-primary" onClick={() => this.toggleModal(false)}>
              Cancel
            </Button>
          </Modal.Footer>
        </Modal>
      </>
    )
  };
}

export default CommissionableDevices;