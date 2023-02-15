import React from 'react';
import { Button, Card, Col, Form, Row, Table } from 'react-bootstrap';
import * as FiIcons from 'react-icons/fi';
import * as RiIcons from 'react-icons/ri';
import { SmartStartProps, SmartStartState } from './smart-start-types';
import { toast } from 'react-toastify';
import PreferredProtocolsDlg from '../../components/preferred-protocols-dlg/preferred-protocols-dlg';
import ConfirmDlg from '../../components/confirm-dlg/confirm-dlg';
import QrCodeDlg from '../../components/qr-code-dlg/qr-code-dlg';
import './smart-start.css';
import { InputAdornment, TextField, Tooltip } from '@mui/material';

export class SmartStart extends React.Component<SmartStartProps, SmartStartState> {
  constructor(props: SmartStartProps) {
    super(props);
    this.state = {
      SmartStartList: this.props.SmartStartList,
      Filter: "",
      ShowModal: false
    };
    this.add = this.add.bind(this);
    this.search = this.search.bind(this);
    this.changePreferredDlg = React.createRef();
    this.changeConfirmDlg = React.createRef();
    this.changeQrCodeDlg = React.createRef();
  }

  changePreferredDlg: any;
  changeConfirmDlg: any;
  changeQrCodeDlg: any;
  itemToRemove: any = {};

  searchEvent(event: any) {
    this.setState({ Filter: event.target.value }, () => this.search(this.props.SmartStartList));
  }

  search(list: any[]) {
    let filtered = list.filter(i => i.DSK.includes(this.state.Filter));
    this.setState({ SmartStartList: filtered });
  }

  add() {
    if (!this.isSomeProcessed(this.state.SmartStartList.length - 1))
      this.setState({ SmartStartList: this.state.SmartStartList.concat({ DSK: "", Include: true, ProtocolControllerUnid: "", Unid: "", IsNew: true }) });
  }

  isSomeProcessed(index: number) {
    if (this.state.SmartStartList.find(i => i.IsError))
      return true;
    let processedRow = this.state.SmartStartList.find(i => i.IsNew || i.IsEdit);
    if (processedRow) {
      this.showError(processedRow);
      return true;
    }
    return false;
  }

  showError(item: any) {
    this.updateState(this.state.SmartStartList.indexOf(item), "IsError", true);
    setTimeout(() => {
      let index = this.state.SmartStartList.indexOf(item);
      if (index > -1)
        this.updateState(index, "IsError", false);
    }, 2400);
  }

  save(item: any) {
    if (!this.isDSKValid(item.DSK)) {
      this.showError(item);
      return;
    }
    delete (item.IsNew);
    delete (item.IsEdit);
    let payload = {
      DSK: item.DSK,
      Unid: item.Unid,
      ProtocolControllerUnid: item.ProtocolControllerUnid,
      Include: item.Include
    };
    this.props.SocketServer.send(JSON.stringify({ type: "run-smart-start-command", data: { Cmd: "Update", Payload: payload } }));
  }

  savePreferred(item: any) {
    let payload = {
      DSK: item.DSK,
      PreferredProtocols: item.PreferredProtocols
    };
    this.props.SocketServer.send(JSON.stringify({ type: "run-smart-start-command", data: { Cmd: "Update", Payload: payload } }));
  }

  isDSKValid(dsk: string) {
    return (/^([0-9a-f-]{1,})$/i).test(dsk);
  }

  edit(index: number) {
    if (!this.isSomeProcessed(index)) {
      this.updateState(index, "IsEdit", true);
    }
  }

  updateStateByEvent(index: number, name: string, event: any) {
    if (name === "DSK" && event.target.value.length > 0 && !this.isDSKValid(event.target.value))
      return;
    this.updateState(index, name, event.target.value);
  }

  updateState(index: number, name: string, value: any) {
    let newList = this.state.SmartStartList;
    newList[index][name] = value;
    this.setState({ SmartStartList: newList });
  }

  editPreferred(item: any) {
    this.changePreferredDlg?.current.updateState(item, true);
  }

  remove(item: any) {
    if (item.IsNew) {
      this.state.SmartStartList.shift();
      this.setState({ SmartStartList: this.state.SmartStartList });
    } else {
      this.itemToRemove = item;
      this.changeConfirmDlg?.current.update(
        `Remove DSK: ${this.itemToRemove.DSK}`,
        `Are you sure, you want to remove DSK from provision list?`,
        this.confirmRemove
      );
    }
  }

  confirmRemove = () => {
    this.props.SocketServer.send(JSON.stringify({ type: "run-smart-start-command", data: { Cmd: "Remove", Payload: { DSK: this.itemToRemove.DSK } } }));
    if (this.itemToRemove.Unid)
      toast("Node will stay in the network and requires to be excluded manually or reset to factory default in order to leave the Z-Wave Network", { type: "warning" });
  }

  cancel() {
    this.props.SocketServer.send(JSON.stringify({ type: "get-smart-start-list" }));
  }

  getField(item: any, fieldName: string, index: number) {
    switch (fieldName) {
      case "DSK":
        return item.IsNew
          ? <TextField value={item.DSK} onChange={this.updateStateByEvent.bind(this, index, 'DSK')} className="flex-input" label="DSK" size="small" type="text" variant="outlined"
            InputProps={{
              endAdornment: <InputAdornment position="end"> <Tooltip title="Scan QR-code">
                <span className="icon">
                  <RiIcons.RiQrScan2Line onClick={() => window.open('https://siliconlabs.github.io/UnifySDK/doc/qr_code_scanner.html', '_blank')} />
                </span>
              </Tooltip> </InputAdornment>
            }}
          />
          : <TextField value={item.DSK} className="flex-input" label="DSK" size="small" type="text" />
      case "Include":
        return item.IsNew || item.IsEdit
          ? <span className="icon">
            {item.Include ? <FiIcons.FiCheckSquare color="#28a745" onClick={this.updateState.bind(this, index, "Include", false)} /> : <FiIcons.FiSquare color="#6c757d" onClick={this.updateState.bind(this, index, "Include", true)} />}
          </span>
          : <>
            <span>
              {item.Include ? <FiIcons.FiCheck color="#28a745" /> : <FiIcons.FiXCircle color="#6c757d" />}
            </span>
            <Tooltip title="Manual Intervention Required" hidden={!item || !item.ManualInterventionRequired}>
              <span className="icon" >
                <FiIcons.FiAlertTriangle color="orange" />
              </span>
            </Tooltip>
          </>
      case "PC UNID":
        return item.IsNew || item.IsEdit
          ? <TextField size="small" className="flex-input" type="text" variant="outlined" label="PC UNID" value={item.ProtocolControllerUnid} onChange={this.updateStateByEvent.bind(this, index, 'ProtocolControllerUnid')} />
          : item.ProtocolControllerUnid
      case "UNID":
        return item.IsNew || item.IsEdit
          ? <TextField size="small" className="flex-input" type="text" variant="outlined" label="UNID" value={item.Unid} onChange={this.updateStateByEvent.bind(this, index, 'Unid')} />
          : item.Unid
      case "Actions":
        return item.IsNew || item.IsEdit
          ? <><Tooltip title="Save">
            <span className="icon">
              <FiIcons.FiSave className="margin-h-5" onClick={this.save.bind(this, item)} />
            </span>
          </Tooltip>
            <Tooltip title="Cancel">
              <span className="icon">
                <FiIcons.FiXCircle color="#dc3545" onClick={this.cancel.bind(this)} />
              </span>
            </Tooltip>
          </>
          : <><Tooltip title="Preferred Protocols">
            <span className="icon">
              <FiIcons.FiHeart className="margin-h-5" onClick={this.editPreferred.bind(this, item)} />
            </span>
          </Tooltip>
            <Tooltip title="Edit">
              <span className="icon">
                <FiIcons.FiEdit className="margin-h-5" onClick={this.edit.bind(this, index)} />
              </span>
            </Tooltip>
            <Tooltip title="Remove">
              <span className="icon">
                <FiIcons.FiTrash2 color="#dc3545" onClick={this.remove.bind(this, item)} />
              </span>
            </Tooltip></>
    }
  }

  render() {
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    return (
      <>
        <h3>The SmartStart List</h3>
        <Row hidden={!this.props.IsConnected} >
          <Col xs={12}>
            <Button variant="outline-primary" className="float-left" onClick={this.add} >Add</Button>
            <Form.Group as={Row} className="float-right">
              <Form.Label column sm="5" className="float-right">Find DSK key:</Form.Label>
              <Col>
                <Form.Control type="text" placeholder="key" onKeyUp={this.searchEvent.bind(this)} />
              </Col>
            </Form.Group>
          </Col>
        </Row>
        {(!this.state.SmartStartList || !this.state.SmartStartList.length || !this.props.IsConnected)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : (isMobile
            ? <div className='table-content smart-start'>
              {
                this.state.SmartStartList.map((item, index) => {
                  return (
                    <Card key={index} className={`${item.IsNew || item.IsEdit ? `editable-tr inline margin-v-10 ${item.IsError ? "blinking-error" : ""}` : "inline margin-v-10"}`}>
                      <Card.Header className='flex'>
                        <div className="col-sm-12 text-right">
                          {this.getField(item, "Actions", index)}
                        </div>
                      </Card.Header>
                      <Card.Body>
                        <div className='col-sm-12'>
                          <div className="col-sm-12 inline padding-v-5">{this.getField(item, "DSK", index)}</div>
                          <div className="col-sm-6 inline padding-v-5"><b hidden={item.IsNew || item.IsEdit}><i>PC UNID: </i></b>{this.getField(item, "PC UNID", index)}</div>
                          <div className="col-sm-6 inline padding-v-5"><b hidden={item.IsNew || item.IsEdit}><i>UNID: </i></b>{this.getField(item, "UNID", index)}</div>
                          <div className="col-sm-6 inline padding-v-5"><b><i>Include: </i></b>{this.getField(item, "Include", index)}</div>
                        </div>
                      </Card.Body>
                    </Card>
                  );
                })}
            </div>
            : <Table striped hover>
              <thead>
                <tr className="">
                  <th className="wd-col-5">DSK</th>
                  <th className="text-center wd-col-1">Include</th>
                  <th className="wd-col-2">PC Unid</th>
                  <th className="wd-col-2">Unid</th>
                  <th className="wd-col-1">&ensp;</th>
                </tr>
              </thead>
              <tbody>
                {this.state.SmartStartList.map((item, index) =>
                  <tr key={index} className={`${item.IsNew || item.IsEdit ? `editable-tr ${item.IsError ? "blinking-error" : ""}` : ""}`}>
                    <td>{this.getField(item, "DSK", index)}
                      {/* {item.IsEdit
                            ? <TextField value={item.DSK} className="flex-input" label="DSK" size="small" type="text" />
                            : <TextField value={item.DSK} onChange={this.updateStateByEvent.bind(this, index, 'DSK')} className="flex-input" label="DSK" size="small" type="text" variant="outlined"
                              InputProps={window.location.protocol === "https:" ? {
                                endAdornment: <InputAdornment position="end"> <Tooltip title="Scan QR-code">
                                  <span className="icon">
                                    <RiIcons.RiQrScan2Line onClick={() => this.changeQrCodeDlg.current?.updateState(true, index)} />
                                  </span>
                                </Tooltip> </InputAdornment>
                              } : {}}
                            />
                          } */}
                    </td>
                    <td className="text-center vertical-middle">
                      {this.getField(item, "Include", index)}
                    </td>
                    <td className={`${item.IsNew || item.IsEdit ? "" : "vertical-middle"}`}>
                      {this.getField(item, "PC UNID", index)}
                    </td>
                    <td className={`${item.IsNew || item.IsEdit ? "" : "vertical-middle"}`}>
                      {this.getField(item, "UNID", index)}
                    </td>
                    <td className="text-center vertical-middle">
                      {this.getField(item, "Actions", index)}
                      {/* <Tooltip title="Save">
                            <span className="icon">
                              <FiIcons.FiSave className="margin-h-5" onClick={this.save.bind(this, item)} />
                            </span>
                          </Tooltip>
                          <Tooltip title="Cancel">
                            <span className="icon">
                              <FiIcons.FiXCircle color="#dc3545" onClick={this.cancel.bind(this)} />
                            </span>
                          </Tooltip> */}
                    </td>
                  </tr>
                )}
              </tbody>
            </Table>)
        }

        <PreferredProtocolsDlg ref={this.changePreferredDlg} SaveFunc={this.savePreferred.bind(this)} />
        <ConfirmDlg ref={this.changeConfirmDlg} />
        <QrCodeDlg ref={this.changeQrCodeDlg} UpdateDSK={this.updateState.bind(this)} />
      </>
    )
  };
}

export default SmartStart;