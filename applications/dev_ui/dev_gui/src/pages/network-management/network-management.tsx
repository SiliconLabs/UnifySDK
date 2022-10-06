import React, { Fragment } from 'react';
import { Button, Card, Col, Dropdown, DropdownButton, Form, InputGroup, Modal, Row, Table } from 'react-bootstrap';
import * as BsIcons from 'react-icons/bs';
import * as RiIcons from 'react-icons/ri';
import { NetworkManagementProps, NetworkManagementState } from './network-management-types';
import { NodeTypesList } from '../../cluster-types/cluster-types';
import { toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import EditableAttribute from '../../components/editable-attribute/editable-attribute';
import { ClusterTypeAttrs } from '../../cluster-types/cluster-type-attributes';
import CommandDlg from '../../components/command-dlg/command-dlg';
import { MenuItem, TextField, Tooltip } from '@mui/material';
import ConfirmDlg from '../../components/confirm-dlg/confirm-dlg';
import StateParamsDlg from '../../components/state-params-dlg/state-params-dlg';

export class NetworkManagement extends React.Component<NetworkManagementProps, NetworkManagementState> {
  constructor(props: NetworkManagementProps) {
    super(props);
    this.state = {
      NodeList: this.props.NodeList,
      ShowSecurityModal: false,
      ShowRemoveModal: false,
      Dsk: "",
      UnidToRemove: "",
      RequestedData: {},
      ProcessingNode: null,
      InclusionProccess: false,
      SortName: "Unid",
      IsSortAcs: true
    };
    this.updateList = this.updateList.bind(this);
    this.handleDskChange = this.handleDskChange.bind(this);
    this.getDSK = this.getDSK.bind(this);
    this.handleKeyPress = this.handleKeyPress.bind(this);
    this.changeCommandDlg = React.createRef();
    this.changeConfirmDlg = React.createRef();
    this.changeStateParamsDlg = React.createRef();
  }
  changeCommandDlg: any;
  changeConfirmDlg: any;
  changeStateParamsDlg: any;
  processedCommand: any;
  ClusterTypeAttrs = ClusterTypeAttrs;

  updateNodeState(node: any) {
    if (this.state.InclusionProccess && node.NetworkStatus === "Online functional") {
      if (!node.Security?.toLocaleLowerCase().includes("s2"))
        toast(`The node "${node.Unid}" is using non-secure communication and could be compromised`, { type: "warning" });
      this.setState({ InclusionProccess: false });
    }
  }

  sendStateParams = (item: any) => {
    this.changeStateParamsDlg.current?.updateState(true, item);
  }

  handleProtocolControllerState(data: any) {
    if (data.State === "add node")
      if (data.RequestedStateParameters && data.RequestedStateParameters.indexOf("SecurityCode") > -1)
        this.setState({ RequestedData: data, ShowSecurityModal: true });
      else
        this.setState({ InclusionProccess: true });
    else if (data.State === "remove node")
      if (data.RequestedStateParameters && data.RequestedStateParameters.indexOf("Unid") > -1)
        this.setState({ RequestedData: data, ShowRemoveModal: true });
  }

  runCommand(unid: string, cmdType: string, payload: any) {
    this.props.SocketServer.send(JSON.stringify({
      type: cmdType,
      data: {
        Unid: unid,
        Payload: payload
      }
    }));
  }

  handleDskChange(event: any) {
    this.setState({ Dsk: event.target.value });
  }

  handleKeyPress(event: any) {
    if (event.key === 'Enter')
      this.setSecurity(true);
  }

  setSecurity(accept: boolean) {
    this.runCommand(this.state.RequestedData.Unid, "run-state-command", {
      State: "add node",
      StateParameters: {
        UserAccept: accept,
        SecurityCode: accept ? (this.state.Dsk + this.getDSK()) : null,
        AllowMultipleInclusions: false
      }
    });
    this.setState({ ShowSecurityModal: false, ProcessingNode: this.state.RequestedData.Unid, Dsk: "" });
  }

  setRemoveData(accept: boolean) {
    this.runCommand(this.state.RequestedData.Unid, "run-state-command", {
      State: "remove node",
      StateParameters: {
        Unid: accept ? this.state.UnidToRemove : null
      }
    });
    this.setState({ ShowRemoveModal: false, UnidToRemove: "" });
  }

  getDSK() {
    let dsk = this.state.RequestedData.StateParameters && this.state.RequestedData.StateParameters.SecurityCode;
    if (dsk)
      return dsk.slice(5);
  }

  runStateCommand(unid: string, cmdType: string, cmd: string) {
    if (cmd === "reset") {
      this.processedCommand = { Unid: unid, CmdType: cmdType, Cmd: cmd };
      this.changeConfirmDlg?.current.update(
        `Reset ${unid}`,
        `Are you sure, you want to reset this Protocol Controller?`
      );
    }
    else
      this.runCommand(unid, cmdType, { State: cmd })
  }

  runConfirmedCommand = () => {
    this.runCommand(this.processedCommand.Unid, this.processedCommand.CmdType, { State: this.processedCommand.Cmd })
  }

  getLocationList = () => {
    let list: Set<string> = new Set<string>();
    this.state.NodeList.forEach(node => node.ep && Object.keys(node.ep).forEach(endPoint => {
      if (node.ep[endPoint].Clusters?.NameAndLocation?.Attributes?.Location?.Reported)
        list.add(node.ep[endPoint].Clusters.NameAndLocation.Attributes.Location.Reported);
    }));
    return [...list];
  }

  setSort = (name: string) => {
    if (this.state.SortName === name)
      this.setState({ IsSortAcs: !this.state.IsSortAcs }, () => this.updateList(this.props.NodeList));
    else
      this.setState({ SortName: name, IsSortAcs: true }, () => this.updateList(this.props.NodeList));
  }

  updateList(list: any[], deviceType?: string) {
    let sortedLst = list.sort((i, j) => {
      return i[this.state.SortName].localeCompare(j[this.state.SortName]);
    });
    if (!this.state.IsSortAcs)
      sortedLst = sortedLst.reverse();
    this.setState({ NodeList: sortedLst });
  }

  render() {
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    return (
      <>
        <h3>Network Management</h3>
        {(!this.state.NodeList || !this.state.NodeList.length)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : (isMobile
            ? <div className='table-content'>
              {this.state.NodeList.map((item, index) => {
                if (!item.ClusterTypes || item.ClusterTypes.indexOf(NodeTypesList.ProtocolController) === -1) return;
                let className = item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable" ? "disabled" : "";
                let title = item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"
                  ? "Node is Offline"
                  : (item.Security?.toLocaleLowerCase().includes("s2") ? "" : "This node using non-secure communication and could be compromised");
                return (
                  <Tooltip key={index} title={title}>
                    <Card key={index} className="inline margin-v-10">
                      <Card.Header className='flex'><div className={`col-sm-6 ${className}`}><b>{item.Unid}</b></div>
                        <div className="col-sm-5 float-right">
                          {item.NetworkManagementState?.SupportedStateList && item.NetworkManagementState.SupportedStateList.length
                            ? <DropdownButton menuAlign={'right'} variant="outline-primary" title="Actions" className="float-right margin-r-5">
                              {item.NetworkManagementState.SupportedStateList.map((cmd: string, cmdIndex: number) => {
                                return (
                                  <Dropdown.Item key={cmdIndex} onClick={() => this.runStateCommand(item.Unid, "run-state-command", cmd)}> {cmd.charAt(0).toUpperCase() + cmd.slice(1)}</Dropdown.Item>
                                )
                              })
                              }
                              <Dropdown.Item onClick={() => this.sendStateParams(item)} hidden={!item.NetworkManagementState.RequestedStateParameters}>Send State Params</Dropdown.Item>
                            </DropdownButton>
                            : <></>}
                        </div>
                      </Card.Header>
                      <Card.Body className={className}>
                        <div className='col-sm-12 flex'>
                          <div className="col-sm-6">
                            <b><i>Name: </i></b><EditableAttribute Node={item} EpName="ep0" Cluster={item.ep?.ep0?.Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
                              SocketServer={this.props.SocketServer} ReplaceNameWithUnid={false} Disabled={item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} />
                          </div>
                          <div className="col-sm-6">
                            <b><i>Location: </i></b><EditableAttribute Node={item} EpName="ep0" Cluster={item.ep?.ep0?.Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Location"
                              SocketServer={this.props.SocketServer} ReplaceNameWithUnid={false} GetOptions={this.getLocationList} Disabled={item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} />
                          </div>
                        </div>
                        <div className='col-sm-12 flex'>
                          <div className="col-sm-6">
                            <div className="flex">
                              <b><i>Status: </i></b> <span hidden={item.NetworkStatus !== "Offline" || item.NetworkStatus === "Unavailable"} className="margin-h-5"><RiIcons.RiWifiOffLine color="red" /></span>
                              <div> {item.NetworkStatus}</div>
                            </div>
                          </div>
                          <div className="col-sm-6"><b><i>State: </i></b>{item.NetworkManagementState?.State}</div>
                        </div>
                        <div className='col-sm-12 flex'>
                          <div className="col-sm-6"><b><i>Supported State List: </i></b>{item.NetworkManagementState?.SupportedStateList
                            ? <ul> {item.NetworkManagementState.SupportedStateList.map((i: string, key: number) => <li key={key}>{i}</li>)}</ul>
                            : "-"}</div>
                          <div className="col-sm-6"><b><i>Requested State Parameters: </i></b>{item.NetworkManagementState?.RequestedStateParameters
                            ? <ul> {item.NetworkManagementState.RequestedStateParameters.map((i: string, key: number) => <li key={key}>{i}</li>)}</ul>
                            : "-"}</div>
                        </div>
                        <div className='col-sm-12 flex'>
                          <div className="col-sm-12"><b><i>State Parameters: </i></b>{item.NetworkManagementState?.StateParameters
                            ? <ul>{Object.keys(item.NetworkManagementState.StateParameters).map((i: any, key: number) => <li key={key}>{i}: {item.NetworkManagementState?.StateParameters[i]}</li>)}</ul>
                            : "-"}</div>
                        </div>
                      </Card.Body>
                    </Card>
                  </Tooltip>
                );
              })}
            </div>
            : <Table>
              <thead>
                <tr>
                  <th>Unid
                    <span onClick={() => this.setSort("Unid")}>
                      {this.state.IsSortAcs
                        ? <BsIcons.BsSortDownAlt size={20} className={`pointer padding-l-5 ${this.state.SortName === "Unid" ? "" : "disabled"}`} />
                        : <BsIcons.BsSortUp size={20} className={`pointer padding-l-5 ${this.state.SortName === "Unid" ? "" : "disabled"}`} />
                      }
                    </span>
                  </th>
                  <th>Name</th>
                  <th>Location</th>
                  <th>State</th>
                  <th>Network Status</th>
                  <th>Supported State List</th>
                  <th>State Parameters</th>
                  <th>Requested State Parameters</th>
                  <th>&ensp;</th>
                </tr>
              </thead>
              <tbody>
                {this.state.NodeList.map((item, index) => {
                  if (!item.ClusterTypes || item.ClusterTypes.indexOf(NodeTypesList.ProtocolController) === -1) return;
                  let tdClassName = item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable" ? "disabled" : "";
                  let title = item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"
                    ? "Node is Offline"
                    : (item.Security?.toLocaleLowerCase().includes("s2") ? "" : "This node using non-secure communication and could be compromised");
                  return (
                    <Tooltip key={index} title={title}>
                      <Fragment>
                        <tr>
                          <td className={tdClassName}>{item.Unid}</td>
                          <td className={tdClassName}>
                            <EditableAttribute Node={item} EpName={"ep0"} Cluster={item.ep?.ep0?.Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
                              SocketServer={this.props.SocketServer} ReplaceNameWithUnid={false} Disabled={item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} />
                          </td>
                          <td className={tdClassName}>
                            <EditableAttribute Node={item} EpName={"ep0"} Cluster={item.ep?.ep0?.Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Location"
                              SocketServer={this.props.SocketServer} ReplaceNameWithUnid={false} GetOptions={this.getLocationList} Disabled={item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} />
                          </td>
                          <td className={tdClassName}>{item.NetworkManagementState.State}</td>
                          <td className={tdClassName}>
                            <div className="flex">
                              <span hidden={item.NetworkStatus !== "Offline" || item.NetworkStatus === "Unavailable"} className="margin-h-5"><RiIcons.RiWifiOffLine color="red" /></span>
                              <div> {item.NetworkStatus}</div>
                            </div>
                          </td>
                          <td className={tdClassName}>{item.NetworkManagementState?.SupportedStateList
                            ? <ul className='padding-l-15'> {item.NetworkManagementState.SupportedStateList.map((i: string, key: number) => <li key={key}>{i}</li>)}</ul>
                            : "-"}</td>
                          <td className={tdClassName}>{item.NetworkManagementState?.StateParameters
                            ? <ul className='padding-l-15'>{Object.keys(item.NetworkManagementState.StateParameters).map((i: any, key: number) => <li key={key}>{i}: {item.NetworkManagementState?.StateParameters[i]}</li>)}</ul>
                            : "-"}</td>
                          <td className={tdClassName}>{item.NetworkManagementState?.RequestedStateParameters
                            ? <ul className='padding-l-15'> {item.NetworkManagementState.RequestedStateParameters.map((i: string, key: number) => <li key={key}>{i}</li>)}</ul>
                            : "-"}</td>
                          <td>
                            {item.NetworkManagementState?.SupportedStateList && item.NetworkManagementState.SupportedStateList.length
                              ? <DropdownButton menuAlign={'right'} variant="outline-primary" title="Actions" className="float-right margin-r-5">
                                {item.NetworkManagementState.SupportedStateList.map((cmd: string, cmdIndex: number) => {
                                  return (
                                    <Dropdown.Item key={cmdIndex} onClick={() => this.runStateCommand(item.Unid, "run-state-command", cmd)}> {cmd.charAt(0).toUpperCase() + cmd.slice(1)}</Dropdown.Item>
                                  )
                                })
                                }
                                <Dropdown.Item onClick={() => this.sendStateParams(item)} hidden={!item.NetworkManagementState.RequestedStateParameters}>Send State Params</Dropdown.Item>
                              </DropdownButton>
                              : <></>}
                          </td>
                        </tr>
                      </Fragment >
                    </Tooltip>
                  );
                })}
              </tbody>
            </Table>)
        }

        <Modal show={this.state.ShowSecurityModal} onHide={this.setSecurity.bind(this, false)}>
          <Modal.Header closeButton>
            <Modal.Title>Secure node inclusion</Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <InputGroup>
              <Form.Control type="text" placeholder="xxxxx" onKeyPress={this.handleKeyPress} value={this.state.Dsk} onChange={this.handleDskChange} maxLength={5} />
              <InputGroup.Text>{this.getDSK()}</InputGroup.Text>
            </InputGroup>
          </Modal.Body>
          <Modal.Footer>
            <Button variant="primary" onClick={this.setSecurity.bind(this, true)}>Send DSK</Button>
            <Button variant="outline-primary" onClick={this.setSecurity.bind(this, false)}>Reject</Button>
          </Modal.Footer>
        </Modal>

        <Modal show={this.state.ShowRemoveModal} onHide={this.setRemoveData.bind(this, false)}>
          <Modal.Header closeButton>
            <Modal.Title>Choice node to remove</Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <TextField size="small" fullWidth={true} select label="UNID" value={this.state.UnidToRemove} defaultValue={this.state.UnidToRemove}
              onChange={(e: any) => this.setState({ UnidToRemove: e.target.value })} variant="outlined">
              {this.props.NodeList.filter(i => i.ClusterTypes.indexOf("ProtocolController") < 0).map((node: any) => {
                return <MenuItem key={node.Unid} value={node.Unid}>
                  {node.Unid}
                </MenuItem>
              })}
            </TextField>
          </Modal.Body>
          <Modal.Footer>
            <Button variant="primary" onClick={this.setRemoveData.bind(this, true)}>Send UNID</Button>
            <Button variant="outline-primary" onClick={this.setRemoveData.bind(this, false)}>Reject</Button>
          </Modal.Footer>
        </Modal>

        <CommandDlg ref={this.changeCommandDlg}
          SocketServer={this.props.SocketServer} />

        <ConfirmDlg ConfirmAction={this.runConfirmedCommand} ref={this.changeConfirmDlg} />
        <StateParamsDlg ref={this.changeStateParamsDlg} SocketServer={this.props.SocketServer} />
      </>
    )
  };
}

export default NetworkManagement;