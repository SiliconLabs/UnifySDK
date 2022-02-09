import React from 'react';
import { Button, Col, Dropdown, DropdownButton, Form, InputGroup, Modal, Row, Table } from 'react-bootstrap';
import * as GrIcons from 'react-icons/gr';
import * as RiIcons from 'react-icons/ri';
import * as MdIcons from 'react-icons/md';
import Tooltip from '@material-ui/core/Tooltip';
import { NodesProps, NodesState } from './nodes-types';
import { NodeTypesList } from '../../cluster-types/cluster-types';
import { ClusterViewOverrides } from '../base-clusters/cluster-view-overrides';
import { toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import './nodes.css';
import ClusterTypeTooltip from '../../components/cluster-type-tooltip/cluster-type-tooltip';
import { MenuItem, TextField } from '@material-ui/core';
import EditableAttribute from '../../components/editable-attribute/editable-attribute';

export class Nodes extends React.Component<NodesProps, NodesState> {
  constructor(props: NodesProps) {
    super(props);
    this.state = {
      NodeList: this.props.NodeList,
      ShowSecurityModal: false,
      ShowRemoveModal: false,
      Dsk: "",
      UnidToRemove: "",
      RequestedData: {},
      Filter: "",
      ProcessingNode: null,
      InclusionProccess: false
    };
    this.search = this.search.bind(this);
    this.handleDskChange = this.handleDskChange.bind(this);
    this.getDSK = this.getDSK.bind(this);
    this.handleKeyPress = this.handleKeyPress.bind(this);
  }

  searchEvent(event: any) {
    this.setState({ Filter: event.target.value }, () => this.search(this.props.NodeList));
  }

  updateNodeState(node: any) {
    if (this.state.InclusionProccess && node.NetworkStatus === "Online functional") {
      if (!node.Security?.toLocaleLowerCase().includes("s2"))
        toast(`The node "${node.Unid}" is using non-secure communication and could be compromised`, { type: "warning" });
      this.setState({ InclusionProccess: false });
    }
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

  search(list: any[]) {
    let filtered = list.filter(i => i.Unid.toLowerCase().includes(this.state.Filter.toLowerCase()));
    this.setState({ NodeList: filtered });
  }

  runStateCommand(unid: string, cmdType: string, cmd: string) {
    this.runCommand(unid, cmdType, { State: cmd })
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

  getTooltip(type: any) {
    if (type === NodeTypesList.ProtocolController)
      return <Tooltip title="Protocol Controller"><span className="icon cursor-defult"><GrIcons.GrNodes /></span></Tooltip>
    return (ClusterViewOverrides as any)[type]?.NodesTooltip || (NodeTypesList as any)[type] || "Unknown";
  }

  getDSK() {
    let dsk = this.state.RequestedData.StateParameters && this.state.RequestedData.StateParameters.SecurityCode;
    if (dsk)
      return dsk.slice(5);
  }

  actionsList = (actions: any[], title: string, runCmd: any, className: string) => <>
    {actions && actions.length
      ? <DropdownButton variant="outline-primary" title={title} className={`float-right ${className}`}>
        {actions.map((cmd: string, cmdIndex: number) => {
          return (
            <Dropdown.Item key={cmdIndex} onClick={() => runCmd(cmd)}> {cmd.charAt(0).toUpperCase() + cmd.slice(1)}</Dropdown.Item>
          )
        })
        }
      </DropdownButton>
      : <></>}
  </>

  getRow = (item: any, index: number, epName: string = "", ep: any = null, indexEp: number = -1, span: number = 1) => {
    let tdClassName = item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable" ? "disabled" : "";
    return (<tr key={`${index}-${indexEp}`}>
      <td className={`${tdClassName} vertical-middle`} rowSpan={span} hidden={indexEp > 0}>{item.Unid}</td>
      <td className={`${tdClassName} vertical-middle`}>{epName}</td>
      <td className={`${tdClassName} flex padding-v-20`} >{(item.ClusterTypes.indexOf(NodeTypesList.ProtocolController) > -1)
        ? <><Tooltip title="Protocol Controller" ><span className="icon cursor-defult"><GrIcons.GrNodes /></span></Tooltip>
          <Tooltip hidden={!item.RFTelemetry} title="RF Telemetry" ><span className="icon cursor-defult"><MdIcons.MdOutlineTransform color="black" /></span></Tooltip></>
        : ep && <ClusterTypeTooltip Ep={[ep]} />}
      </td>
      <td className={`${tdClassName} vertical-middle`}>
        {item.ClusterTypes.indexOf(NodeTypesList.ProtocolController) > -1
          ? ""
          : <EditableAttribute Unid={`${item.Unid}/${epName}`} Cluster={ep?.Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name" SocketServer={this.props.SocketServer} ReplaceNameWithUnid={false} />}
      </td>
      <td className={`${tdClassName} vertical-middle`}>
        {item.ClusterTypes.indexOf(NodeTypesList.ProtocolController) > -1
          ? ""
          : <EditableAttribute Unid={`${item.Unid}/${epName}`} Cluster={ep?.Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Location" SocketServer={this.props.SocketServer} ReplaceNameWithUnid={false} />}
      </td>
      <td className={`${tdClassName} vertical-middle`} rowSpan={span} hidden={indexEp > 0}>
        <div className="flex">
          <span hidden={item.NetworkStatus !== "Offline" || item.NetworkStatus === "Unavailable"} className="margin-h-5"><RiIcons.RiWifiOffLine color="red" /></span>
          <div> {item.NetworkStatus}</div>
        </div>
      </td>
      <td className={`${tdClassName} vertical-middle`} rowSpan={span} hidden={indexEp > 0}>
        <div className="flex">
          <div>
            <div className={`dot-icon ${item.Security?.toLocaleLowerCase().includes("s2") ? "s2-security" : (item.Security?.toLocaleLowerCase().includes("s0") ? "s0-security" : "none-security")}`}></div>
          </div>
          <div>{item.Security}</div>
        </div>
      </td>
      <td className={`${tdClassName} vertical-middle`} rowSpan={span} hidden={indexEp > 0}>{item.MaximumCommandDelay}</td>
      <td className={`${tdClassName} vertical-middle`} rowSpan={span} hidden={indexEp > 0}>{item.State}</td>
      <td className={`${tdClassName} vertical-middle`} rowSpan={span} hidden={indexEp > 0}>
        {this.actionsList(item.SupportedCommands, "Commands", this.runCommand.bind(this, item.Unid, "run-node-command"), "")}
        {this.actionsList(item.SupportedStateList, "States", this.runStateCommand.bind(this, item.Unid, "run-state-command"), "margin-r-5")}
      </td>
    </tr>)
  }

  getEpRows = (item: any, index: number) => {
    let endPoints = Object.keys(item.ep);
    return (<>
      {endPoints.map((endPoint: any, indexEp: number) => {
        return this.getRow(item, index, endPoint, item.ep[endPoint], indexEp, endPoints.length)
      })}
    </>)
  }

  render() {
    return (
      <>
        <h3>Node List</h3>
        <Row hidden={!this.props.IsConnected} >
          <Col xs={12}>
            <Form.Group as={Row} className="float-right">
              <Form.Label column sm="5" className="float-right">Find Unid key:</Form.Label>
              <Col>
                <Form.Control type="text" placeholder="key" onKeyUp={this.searchEvent.bind(this)} />
              </Col>
            </Form.Group>
          </Col>
        </Row>
        {(!this.state.NodeList || !this.state.NodeList.length)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table>
            <thead>
              <tr className="">
                <th>Unid</th>
                <th>Ep</th>
                <th>Type</th>
                <th>Name</th>
                <th>Location</th>
                <th>Status</th>
                <th>Security</th>
                <th>Max Delay</th>
                <th>State</th>
                <th>&ensp;</th>
              </tr>
            </thead>
            <tbody>
              {this.state.NodeList.map((item, index) => {
                let title = item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"
                  ? "Node is Offline"
                  : (item.Security?.toLocaleLowerCase().includes("s2") ? "" : "This node using non-secure communication and could be compromised");
                return (
                  <Tooltip key={index} title={title}>
                    {
                      !item.ep
                        ? this.getRow(item, index)
                        : this.getEpRows(item, index)
                    }

                  </Tooltip>
                );
              })}
            </tbody>
          </Table>
        }

        <Modal show={this.state.ShowSecurityModal} onHide={this.setSecurity.bind(this, false)}>
          <Modal.Header closeButton>
            <Modal.Title>Secure node inclusion</Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <InputGroup>
              <Form.Control type="text" placeholder="xxxxx" onKeyPress={this.handleKeyPress} value={this.state.Dsk} onChange={this.handleDskChange} maxLength={5} />
              <InputGroup.Prepend>
                <InputGroup.Text>{this.getDSK()}</InputGroup.Text>
              </InputGroup.Prepend>
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
              onChange={(e) => this.setState({ UnidToRemove: e.target.value })} variant="outlined">
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
      </>
    )
  };
}

export default Nodes;