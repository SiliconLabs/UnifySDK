import React from 'react';
import { Button, Col, Form, Modal, Row, Spinner, Table } from 'react-bootstrap';
import * as FiIcons from 'react-icons/fi';
import * as CgIcons from 'react-icons/cg';
import { UPTIListProps, UPTIListState, UPTIItem } from './upti-list-types';
import { Link, useHistory } from 'react-router-dom';
import { Tooltip } from '@mui/material';

export class UPTI extends React.Component<UPTIListProps, UPTIListState> {
  constructor(props: UPTIListProps) {
    super(props);
    this.state = {
      IsDiscover: false,
      ShowModal: false,
      EnablePTI: {
        SerialNumber: "",
        OTAEnable: false,
        AdditionalDataEnable: false
      }
    };
  }

  discoverCompleted() {
    this.setState({ IsDiscover: false });
  }

  discover() {
    this.setState({ IsDiscover: true }, () => this.props.SocketServer.send(JSON.stringify({
      type: "discovery-upti",
      data: null
    })));
  }

  togglePTI(item: UPTIItem) {
    if (item.Enabled)
      this.sendTogglePTI(item.SerialNumber);
    else {
      let pti = this.state.EnablePTI;
      pti.SerialNumber = item.SerialNumber;
      this.setState({ EnablePTI: pti, ShowModal: true });
    }
  }

  viewTrace = () => {
    let history = useHistory();
    history.push(`/nodes`);
  }

  toggleModal(value: boolean) {
    if (value)
      this.sendTogglePTI(this.state.EnablePTI.SerialNumber, true, this.state.EnablePTI.AdditionalDataEnable, this.state.EnablePTI.OTAEnable);
    this.setState({ ShowModal: false });
  }

  handleChange(event: any) {
    let enablePTI: any = this.state.EnablePTI;
    enablePTI[event.target.name] = event.target.checked;
    this.setState({ EnablePTI: enablePTI });
  }

  sendTogglePTI(serialNumber: string, enable: boolean = false, getAddData: boolean = false, getOTA: boolean = false) {
    this.props.SocketServer.send(JSON.stringify({
      type: "toggle-upti",
      data: {
        SerialNumber: serialNumber,
        Payload: {
          Enable: enable,
          GetOTA: getOTA,
          GetAdditionalData: getAddData
        }
      }
    }));
  }

  render() {
    return (
      <>
        <h3>The UPTI List</h3>
        <Row hidden={!this.props.IsConnected} >
          <Col xs={12}>
            <Button disabled={this.state.IsDiscover} variant="outline-primary" className="float-left margin-b-10" onClick={() => this.discover()}>Discover</Button>
            <Spinner hidden={!this.state.IsDiscover} as="span" animation="border" size="sm" variant="primary" className="margin-v-10 margin-h-5" />
          </Col>
        </Row>
        {(!this.props.UPTI.List || !this.props.UPTI.List.length || !this.props.IsConnected)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table striped hover>
            <thead>
              <tr>
                <th>Serial Number</th>
                <th>IP Address</th>
                <th>Enabled</th>
                <th>&ensp;</th>
              </tr>
            </thead>
            <tbody>
              {this.props.UPTI.List.map((item, index) => {
                return (
                  <tr key={index}>
                    <td>{item.SerialNumber}</td>
                    <td>{item.PTIAddress}</td>
                    <td>
                      <span>
                        {item.Enabled ? <FiIcons.FiCheck color="#28a745" /> : <FiIcons.FiXCircle color="#6c757d" />}
                      </span>
                    </td>
                    <td>
                      <Tooltip title={item.Enabled ? "Disable" : "Enable"} className="float-right">
                        <span className="icon">
                          {item.Enabled ? <FiIcons.FiStopCircle color="#dc3545" className="margin-h-5" onClick={this.togglePTI.bind(this, item)} />
                            : <FiIcons.FiPlayCircle color="#28a745" className="margin-h-5" onClick={this.togglePTI.bind(this, item)} />}
                        </span>
                      </Tooltip>
                      <Tooltip title="View Trace" hidden={!item.Enabled} className="float-right">
                        <span className="icon">
                          <Link to={`/upti/${item.SerialNumber}`}><CgIcons.CgDebug className="margin-h-5" /></Link>
                        </span>
                      </Tooltip>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </Table>
        }

        <Modal show={this.state.ShowModal} onHide={() => this.toggleModal(false)}>
          <Modal.Header closeButton>
            <Modal.Title>Enable PTI</Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <Form.Label column sm="12">
              <div className="check-container">
                <Form.Check name="AdditionalDataEnable" defaultChecked={this.state.EnablePTI.AdditionalDataEnable} onChange={this.handleChange.bind(this)} />
              </div>
              Parse Additional Data</Form.Label>
            {/* <Form.Label column sm="12">
              <div className="check-container">
                <Form.Check name="OTAEnable" defaultChecked={this.state.EnablePTI.OTAEnable} onChange={this.handleChange.bind(this)} />
              </div>
              Parse OTA</Form.Label> */}
          </Modal.Body>
          <Modal.Footer>
            <Button variant="primary" onClick={() => this.toggleModal(true)}>Enable</Button>
            <Button variant="outline-primary" onClick={() => this.toggleModal(false)}>Cancel</Button>
          </Modal.Footer>
        </Modal>
      </>
    )
  };
}

export default UPTI;