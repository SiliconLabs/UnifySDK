import React, { Fragment } from 'react';
import { Button, Col, Dropdown, DropdownButton, Form, InputGroup, Modal, Row, Table } from 'react-bootstrap';
import * as GrIcons from 'react-icons/gr';
import * as BsIcons from 'react-icons/bs';
import * as RiIcons from 'react-icons/ri';
import * as MdIcons from 'react-icons/md';
import { DeviceTypes, NodesProps, NodesState } from './nodes-types';
import { NodeTypesList } from '../../cluster-types/cluster-types';
import { ClusterViewOverrides } from '../base-clusters/cluster-view-overrides';
import { toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import './nodes.css';
import ClusterTypeTooltip from '../../components/cluster-type-tooltip/cluster-type-tooltip';
import EditableAttribute from '../../components/editable-attribute/editable-attribute';
import { Link } from 'react-router-dom';
import { ClusterTypeAttrs } from '../../cluster-types/cluster-type-attributes';
import CommandDlg from '../../components/command-dlg/command-dlg';
import { MenuItem, TextField, Tooltip } from '@mui/material';
import ConfirmDlg from '../../components/confirm-dlg/confirm-dlg';

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
      InclusionProccess: false,
      SortName: "Unid",
      IsSortAcs: true,
      DeviceTypes: DeviceTypes,
      DeviceType: "Any",
      IsAllExpanded: false
    };
    this.search = this.search.bind(this);
    this.handleDskChange = this.handleDskChange.bind(this);
    this.getDSK = this.getDSK.bind(this);
    this.handleKeyPress = this.handleKeyPress.bind(this);
    this.changeCommandDlg = React.createRef();
    this.changeConfirmDlg = React.createRef();
  }
  changeCommandDlg: any;
  changeConfirmDlg: any;
  processedCommand: any;
  ClusterTypeAttrs = ClusterTypeAttrs;

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

  search(list: any[], deviceType?: string) {
    let filtered: any[] = [];
    let deviceTypes: Map<string, string> = DeviceTypes;
    deviceType = deviceType || this.state.DeviceType;
    list.forEach(i => {
      let deviceFilter = [...DeviceTypes].find(([k, v]) => v === deviceType);
      let devType = (deviceFilter && deviceFilter[0]) || deviceType?.toLowerCase();
      if ((!deviceType || deviceType === "Any" || i.Unid.toLowerCase().startsWith(devType)) && (i.Unid.toLowerCase().includes(this.state.Filter.toLowerCase())
        || (
          i.ep !== undefined && Object.keys(i.ep).filter(
            (ep: any) => i.ep[ep].Clusters?.NameAndLocation?.Attributes?.Name?.Reported?.toLowerCase().includes(this.state.Filter.toLowerCase())
              || i.ep[ep].Clusters?.NameAndLocation?.Attributes?.Location?.Reported?.toLowerCase().includes(this.state.Filter.toLowerCase())).length > 0
        ))) {
        if (this.state.NodeList.find(n => n.Unid === i.Unid)?.IsExpanded)
          i.IsExpanded = true;
        filtered.push(i);
      }
      let parsedUnid = i.Unid.split("-");
      if (parsedUnid.length > 1 && !deviceTypes.has(parsedUnid[0].toLowerCase()))
        deviceTypes.set(parsedUnid[0].toLowerCase(), parsedUnid[0].toUpperCase());
    });

    filtered = filtered.sort((i, j) => {
      return i[this.state.SortName].localeCompare(j[this.state.SortName]);
    });
    if (!this.state.IsSortAcs)
      filtered = filtered.reverse();
    this.setState({ NodeList: filtered, DeviceTypes: deviceTypes, DeviceType: deviceType });
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
      return <Tooltip title="Protocol Controller"><span className="icon cursor-default"><GrIcons.GrNodes /></span></Tooltip>
    return (ClusterViewOverrides as any)[type]?.NodesTooltip || (NodeTypesList as any)[type] || "Unknown";
  }

  getLocationList = () => {
    let list: Set<string> = new Set<string>();
    this.state.NodeList.forEach(node => node.ep && Object.keys(node.ep).forEach(endPoint => {
      if (node.ep[endPoint].Clusters?.NameAndLocation?.Attributes?.Location?.Reported)
        list.add(node.ep[endPoint].Clusters.NameAndLocation.Attributes.Location.Reported);
    }));
    return [...list];
  }

  getDSK() {
    let dsk = this.state.RequestedData.StateParameters && this.state.RequestedData.StateParameters.SecurityCode;
    if (dsk)
      return dsk.slice(5);
  }

  toggleIsAllExpanded = () => {
    this.state.NodeList.forEach(item =>
      item.IsExpanded = !this.state.IsAllExpanded
    );
    this.setState({ IsAllExpanded: !this.state.IsAllExpanded });
  }

  actionsList = (actions: any[], title: string, runCmd: any, className: string, variant: string = "outline-primary", isSmall: boolean = false) => <>
    {actions && actions.length
      ? <DropdownButton variant={variant} title={title} {...(isSmall ? { size: "sm" } : {})} className={`float-right ${className}`}>
        {actions.map((cmd: string, cmdIndex: number) => {
          return (
            <Dropdown.Item key={cmdIndex} onClick={() => runCmd(cmd)}> {cmd.charAt(0).toUpperCase() + cmd.slice(1)}</Dropdown.Item>
          )
        })
        }
      </DropdownButton>
      : <></>}
  </>

  getRow = (item: any, index: number, epName: string = "", ep: any = null, indexEp: number = -1, endPointsCount: number = 1, clustersCount: number = 0) => {
    let tdClassName = item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable" ? "disabled" : "";

    return (<Fragment key={`${index}-${indexEp}`}>
      <tr>
        <td className={`${tdClassName} no-padding-h`} rowSpan={endPointsCount + clustersCount} hidden={indexEp > 0}>
          <span className='icon small' onClick={() => item.IsExpanded = !item.IsExpanded} hidden={item.ClusterTypes.indexOf(NodeTypesList.ProtocolController) > -1}>
            {item.IsExpanded ? <BsIcons.BsDashSquare /> : <BsIcons.BsPlusSquare />}</span>
        </td>
        <td className={tdClassName} rowSpan={endPointsCount + clustersCount} hidden={indexEp > 0}>{item.Unid}</td>
        <td className={tdClassName} rowSpan={item.IsExpanded ? 1 + (ep?.Clusters ? Object.keys(ep?.Clusters).filter(f => (ClusterViewOverrides as any)[f]?.IsExpandable).length : 0) : 1}>{epName}</td>
        <td className={`${tdClassName} flex`} >{(item.ClusterTypes.indexOf(NodeTypesList.ProtocolController) > -1)
          ? <><Tooltip title="Protocol Controller" ><span className="icon cursor-default"><GrIcons.GrNodes /></span></Tooltip>
            <Tooltip hidden={!item.RFTelemetry} title="RF Telemetry" ><span className="icon cursor-default"><Link to={`/rftelemetry`}><MdIcons.MdOutlineTransform color="black" /></Link></span></Tooltip></>
          : ep && <ClusterTypeTooltip Ep={[ep]} />}
        </td>
        <td className={tdClassName}>
          <EditableAttribute Node={item} EpName={epName} Cluster={ep?.Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
            SocketServer={this.props.SocketServer} ReplaceNameWithUnid={false} Disabled={item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} />
        </td>
        <td className={tdClassName}>
          <EditableAttribute Node={item} EpName={epName} Cluster={ep?.Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Location"
            SocketServer={this.props.SocketServer} ReplaceNameWithUnid={false} GetOptions={this.getLocationList} Disabled={item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} />
        </td>
        <td className={tdClassName} rowSpan={item.IsExpanded ? 1 : endPointsCount} hidden={!item.IsExpanded && indexEp > 0}>
          <div className="flex">
            <span hidden={item.NetworkStatus !== "Offline" || item.NetworkStatus === "Unavailable"} className="margin-h-5"><RiIcons.RiWifiOffLine color="red" /></span>
            <div> {item.NetworkStatus}</div>
          </div>
        </td>
        <td className={tdClassName} rowSpan={item.IsExpanded ? 1 : endPointsCount} hidden={!item.IsExpanded && indexEp > 0}>
          <div className="flex">
            <div>
              <div className={`dot-icon ${item.Security?.toLocaleLowerCase().includes("s2") ? "s2-security" : (item.Security?.toLocaleLowerCase().includes("s0") ? "s0-security" : "none-security")}`}></div>
            </div>
            <div>{item.Security}</div>
          </div>
        </td>
        <td className={tdClassName} rowSpan={item.IsExpanded ? 1 : endPointsCount} hidden={!item.IsExpanded && indexEp > 0}>{item.MaximumCommandDelay}</td>
        <td className={tdClassName} rowSpan={item.IsExpanded ? 1 : endPointsCount} hidden={!item.IsExpanded && indexEp > 0}>{item.State}</td>
        <td rowSpan={item.IsExpanded ? 1 : endPointsCount} hidden={!item.IsExpanded && indexEp > 0}>
          {this.actionsList(item.SupportedCommands, "Commands", this.runCommand.bind(this, item.Unid, "run-node-command"), "")}
          {this.actionsList(item.SupportedStateList, "States", this.runStateCommand.bind(this, item.Unid, "run-state-command"), "margin-r-5")}
        </td>
      </tr>
      {
        item.IsExpanded && ep?.Clusters !== undefined
          ? Object.keys(ep.Clusters).map((cluster, clIndex) => {
            if (!(ClusterViewOverrides as any)[cluster]?.IsExpandable)
              return null;
            let tableRow = (ClusterViewOverrides as any)[cluster]?.ViewTable;
            return (
              <tr key={`${index}-${indexEp}-${clIndex}`} className={`${tdClassName} cluster-info`}>
                <td><b>{cluster}</b></td>
                {[...Array(6).keys()].map((i, iKey) => {
                  let attributes = ep.Clusters[cluster].Attributes && Object.keys(ep.Clusters[cluster].Attributes);
                  return (
                    <td key={`${index}-${indexEp}-${clIndex}-${iKey}`}>
                      {tableRow
                        ? (tableRow[i] ? <span><b><i>{tableRow[i].Name}</i></b>: {tableRow[i].Value(ep.Clusters[cluster], undefined, ep.Clusters)}</span> : null)
                        : <span>{attributes && attributes[i] !== undefined && attributes[i] !== "ClusterRevision" ? <span><b><i>{attributes[i]}</i></b>: {JSON.stringify(ep.Clusters[cluster].Attributes[attributes[i]]?.Reported || "")} </span> : null}</span>
                      }
                    </td>
                  )
                }
                )}
                <td className="buttons-group">
                  {this.actionsList(ep.Clusters[cluster].SupportedCommands && ep.Clusters[cluster].SupportedCommands.filter((cmd: any) => cmd !== "WriteAttributes"), "Commands", this.preSendCommand.bind(this, item, epName, cluster), "", "outline-secondary", true)}
                </td>
              </tr>)
          })
          : null
      }
    </Fragment >
    )
  }


  preSendCommand(node: any, epName: string, clusterType: string, cmd: string) {
    let command = this.ClusterTypeAttrs[clusterType].server.commands.find((i: { name: string; }) => i.name === cmd);
    if (!command)
      return;

    if (command && command.fields && command.fields.length) {
      this.changeCommandDlg.current.updateState(`${node.Unid}/${epName}`, command, true, clusterType);
    } else
      this.props.SocketServer.send(JSON.stringify(
        {
          type: "run-cluster-command",
          data: {
            Unid: `${node.Unid}/${epName}`,
            ClusterType: clusterType,
            Cmd: command.name,
            Payload: {}
          }
        }));
  }

  getEndPoints = (item: any, index: number) => {
    let endPoints = Object.keys(item.ep);
    let clustersCount = item.IsExpanded ? endPoints.reduce((i, j: any) => (i + (item.ep[j].Clusters !== undefined ? Object.keys(item.ep[j].Clusters).filter(f => (ClusterViewOverrides as any)[f]?.IsExpandable).length : 0)), 0) : 0;
    return (<>
      {endPoints.map((endPoint: any, indexEp: number) => {
        return this.getRow(item, index, endPoint, item.ep[endPoint], indexEp, endPoints.length, clustersCount);
      })}
    </>)
  }

  sort = (name: string) => {
    if (this.state.SortName === name)
      this.setState({ IsSortAcs: !this.state.IsSortAcs }, () => this.search(this.props.NodeList));
    else
      this.setState({ SortName: name, IsSortAcs: true }, () => this.search(this.props.NodeList));
  }

  handleDeviceTypeChange = (event: any) => {
    this.search(this.props.NodeList, event.target.value)
  }

  render() {
    return (
      <>
        <h3>Node List</h3>
        <Row hidden={!this.props.IsConnected} >
          <Col xs={12}>
            <TextField className="float-left flex-input col-sm-2" select label="Device Type" size="small" defaultValue="Any" disabled={this.state.DeviceTypes.size === 1}
              onChange={this.handleDeviceTypeChange} variant="outlined" >
              {[...this.state.DeviceTypes].map((option: any) => {
                return <MenuItem key={option[0]} value={option[0]} className={option[0] === "Any" ? "disabled" : ""}>
                  {option[1]}
                </MenuItem>
              })}
            </TextField>
            <Tooltip title="Search by 'Unid', 'Name' or 'Location'">
              <Form.Group as={Row} className="float-right">
                <Form.Label column sm="3" className="float-right">Search:</Form.Label>
                <Col>
                  <Form.Control type="text" placeholder="Search" onKeyUp={this.searchEvent.bind(this)} />
                </Col>
              </Form.Group>
            </Tooltip>
          </Col>
        </Row>
        {(!this.state.NodeList || !this.state.NodeList.length)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table className='nodes-table'>
            <thead>
              <tr className="">
                <th className="no-padding-h">
                  <span className='icon small' onClick={this.toggleIsAllExpanded}>
                    {this.state.IsAllExpanded ? <BsIcons.BsDashSquare /> : <BsIcons.BsPlusSquare />}
                  </span>
                </th>
                <th>Unid
                  <span onClick={() => this.sort("Unid")}>
                    {this.state.IsSortAcs
                      ? <BsIcons.BsSortDownAlt size={20} className={`pointer padding-l-5 ${this.state.SortName === "Unid" ? "" : "disabled"}`} />
                      : <BsIcons.BsSortUp size={20} className={`pointer padding-l-5 ${this.state.SortName === "Unid" ? "" : "disabled"}`} />
                    }
                  </span>
                </th>
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
                        : this.getEndPoints(item, index)
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
      </>
    )
  };
}

export default Nodes;