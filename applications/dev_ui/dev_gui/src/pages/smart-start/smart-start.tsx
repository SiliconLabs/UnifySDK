import React from 'react';
import { Button, Col, Form, Row, Table } from 'react-bootstrap';
import * as FiIcons from 'react-icons/fi';
import * as RiIcons from 'react-icons/ri';
import { SmartStartProps, SmartStartState } from './smart-start-types';
import Tooltip from '@material-ui/core/Tooltip';
import { toast } from 'react-toastify';
import PreferredProtocolsDlg from '../../components/preferred-protocols-dlg/preferred-protocols-dlg';
import ConfirmDlg from '../../components/confirm-dlg/confirm-dlg';
import QrCodeDlg from '../../components/qr-code-dlg/qr-code-dlg';
import { InputAdornment, TextField } from '@material-ui/core';
import './smart-start.css';

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
    this.props.SocketServer.send(JSON.stringify({ type: "run-smart-start-command", data: { Cmd: "Update", Payload: item } }));
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
        `Are you sure, you want to remove DSK from provision list?`
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

  render() {
    return (
      <>
        <h3>The Smart Start List</h3>
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
              {this.state.SmartStartList.map((item, index) => {
                if (item.IsNew || item.IsEdit)
                  return (
                    <tr key={index} className={`editable-tr ${item.IsError ? "blinking-error" : ""}`}>
                      <td className={item.IsEdit ? "" : "padding-5"}>
                        {item.IsEdit
                          ? <TextField value={item.DSK} className="flex-input" size="small" type="text" />
                          : <TextField value={item.DSK} onChange={this.updateStateByEvent.bind(this, index, 'DSK')} className="flex-input" label="DSK" size="small" type="text" variant="outlined"
                            // InputProps={{
                            //   endAdornment: <InputAdornment position="end"> <Tooltip title="Scan QR-code">
                            //     <span className="icon">
                            //       <RiIcons.RiQrScan2Line onClick={() => this.changeQrCodeDlg.current?.updateState(true, index)} />
                            //     </span>
                            //   </Tooltip> </InputAdornment>
                            // }}
                             />
                        }
                      </td>
                      <td className="text-center">
                        <span className="icon">
                          {item.Include ? <FiIcons.FiCheckSquare color="#28a745" onClick={this.updateState.bind(this, index, "Include", false)} /> : <FiIcons.FiSquare color="#6c757d" onClick={this.updateState.bind(this, index, "Include", true)} />}
                        </span>
                      </td>
                      <td className={item.IsEdit ? "" : "padding-5"}>
                        {item.IsEdit
                          ? item.ProtocolControllerUnid
                          : <TextField size="small" className="flex-input" type="text" variant="outlined" label="PC UNID" value={item.ProtocolControllerUnid} onChange={this.updateStateByEvent.bind(this, index, 'ProtocolControllerUnid')} />
                        }
                      </td>
                      <td className="padding-5">
                        <TextField size="small" className="flex-input" type="text" variant="outlined" label="UNID" value={item.Unid} onChange={this.updateStateByEvent.bind(this, index, 'Unid')} />
                      </td>
                      <td className="text-center">
                        <Tooltip title="Save">
                          <span className="icon">
                            <FiIcons.FiSave className="margin-h-5" onClick={this.save.bind(this, item)} />
                          </span>
                        </Tooltip>
                        <Tooltip title="Cancel">
                          <span className="icon">
                            <FiIcons.FiXCircle color="#dc3545" onClick={this.cancel.bind(this)} />
                          </span>
                        </Tooltip>
                      </td>
                    </tr>
                  );
                else return (
                  <tr key={index}>
                    <td><TextField value={item.DSK} size="small" className="flex-input" type="text" /></td>
                    <td className="text-center">
                      <span>
                        {item.Include ? <FiIcons.FiCheck color="#28a745" /> : <FiIcons.FiXCircle color="#6c757d" />}
                      </span>
                    </td>
                    <td>{item.ProtocolControllerUnid}</td>
                    <td>{item.Unid}</td>
                    <td className="text-center">
                      <Tooltip title="Preferred Protocols">
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
                      </Tooltip>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </Table>
        }

        <PreferredProtocolsDlg ref={this.changePreferredDlg} SaveFunc={this.save.bind(this)} />
        <ConfirmDlg ref={this.changeConfirmDlg} ConfirmAction={this.confirmRemove} />
        <QrCodeDlg ref={this.changeQrCodeDlg} UpdateDSK={this.updateState.bind(this)} />
      </>
    )
  };
}

export default SmartStart;