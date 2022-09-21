import React, { Fragment } from 'react';
import * as FiIcons from 'react-icons/fi';
import * as BsIcons from 'react-icons/bs';
import { Button, Col, Dropdown, DropdownButton, Modal, Row, Table } from 'react-bootstrap';
import EditableAttribute from '../../components/editable-attribute/editable-attribute';
import CommandDlg from '../../components/command-dlg/command-dlg';
import { ClusterTypeAttrs } from '../../cluster-types/cluster-type-attributes';
import { MenuItem, TextField, Tooltip } from '@mui/material';
import { BindingProps, BindingState } from './binding-types';


export class Binding extends React.Component<BindingProps, BindingState> {
  constructor(props: BindingProps) {
    super(props);
    this.state = {
      List: this.getList(this.props.NodeList),
      IsAllExpanded: false,
      ShowBindModel: false,
      ShowUnBindModel: false,
      BindCmd: { DestinationUnid: "", DestinationEp: "" },
      UnBindCluster: {}
    }
    this.changeCommandDlg = React.createRef();
  }

  changeCommandDlg: any;
  unBindItem: any;

  preSendCommand(unid: string, endPoint: string, cmd: string) {
    let command = ClusterTypeAttrs["Binding"].server.commands.find((i: { name: string; }) => i.name === cmd);
    if (!command)
      return;

    if (command && command.fields && command.fields.length) {
      this.changeCommandDlg.current.updateState(`${unid}/${endPoint}`, command, true);
    } else
      this.sendCommand(unid, endPoint, command.name, {})
  }

  sendCommand = (unid: string, endPoint: string, name: string, payload: any) => {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "run-cluster-command",
        data: {
          Unid: `${unid}/${endPoint}`,
          ClusterType: "Binding",
          Cmd: name,
          Payload: payload
        }
      }));
  }

  preUnBindCommand = (item: any) => {
    this.unBindItem = item;
    this.setState({ ShowUnBindModel: true, UnBindCluster: {} });
  }

  unBindCommand = () => {
    this.sendCommand(this.unBindItem.Node.Unid, this.unBindItem.EndPoint, "Unbind",
      {
        ClusterName: this.state.UnBindCluster.ClusterName,
        DestinationUnid: this.state.UnBindCluster.DestinationUnid,
        DestinationEp: this.state.UnBindCluster.DestinationEp
      }
    )
    this.unBindItem = null;
    this.setState({ UnBindCluster: null, ShowUnBindModel: false })
  }

  handleUnBindClusterChange = (event: any) => {
    this.setState({ UnBindCluster: event.target.value });
  }

  preBindCommand = (item: any) => {
    this.setState({ BindCmd: { Unid: item.Node.Unid, EndPoint: item.EndPoint }, ShowBindModel: true });
  }

  bindCommand = () => {
    if (this.state.BindCmd.ClusterName && this.state.BindCmd.DestinationUnid && this.state.BindCmd.DestinationEp) {
      this.sendCommand(this.state.BindCmd.Unid, this.state.BindCmd.EndPoint, "Bind", {
        ClusterName: this.state.BindCmd.ClusterName,
        DestinationUnid: this.state.BindCmd.DestinationUnid,
        DestinationEp: Number(this.state.BindCmd.DestinationEp.slice(2))
      });
      this.setState({ ShowBindModel: false });
    }
  }

  handleChange = (event: any) => {
    let bindCmd = this.state.BindCmd;
    bindCmd[event.target.name] = event.target.value;
    if (event.target.name === "DestinationUnid")
      bindCmd.DestinationEp = "";
    this.setState({ BindCmd: bindCmd });
  }

  updateState(nodeList: any) {
    this.setState({ List: this.getList(nodeList) });
  }

  getList(nodeList: any) {
    let list = new Map<string, any>();
    nodeList.forEach((item: any) => {
      if (!item.ep)
        return;
      Object.keys(item.ep).forEach(ep => {
        if (!item.ep[ep].Clusters?.Binding)
          return;
        let bindingTable = this.getBindingTable(item.ep[ep].Clusters?.Binding?.Attributes?.BindingTable?.Reported, item.ep[ep].Clusters?.Binding?.Attributes?.BindingTable?.Desired);
        list.set(item.Unid + ep, {
          Node: item,
          EndPoint: ep,
          BindingTableFull: item.ep[ep].Clusters?.Binding?.Attributes?.BindingTableFull?.Reported,
          BindableClusterList: item.ep[ep].Clusters?.Binding?.Attributes?.BindableClusterList?.Reported,
          BindingTable: bindingTable,
          IsExpanded: (this.state && this.state.List && this.state.List.get(item.Unid + ep)?.IsExpanded) ?? bindingTable?.length
        });
      });
    });
    return list;
  }

  getBindingTable = (reported: any, desired: any) => {
    let table = reported && reported.length ? reported.map((i: any) => {
      let item = { ...i };
      if (!desired || !desired.length || !desired.find((d: any) => d.DestinationUnid === item.DestinationUnid && d.DestinationEp === item.DestinationEp && d.ClusterName === item.ClusterName)) {
        item.color = "#dc3545";
        item.title = "Pending to be removed";
      }
      else {
        item.color = "#212529";
        item.title = "";
      }
      return item;
    }) : [];
    desired?.forEach((item: any) => {
      if (!reported || !reported.length || !reported.find((r: any) => r.DestinationUnid === item.DestinationUnid && r.DestinationEp === item.DestinationEp && r.ClusterName === item.ClusterName)) {
        let desiredItem = { ...item };
        desiredItem.color = "#28a745";
        desiredItem.title = "Pending to be added";
        table.push(desiredItem);
      }
    });
    return this.sortArray(table);
  }

  getRow = (item: any, cluster: any) => {
    return (
      <>
        <td style={{ color: cluster.color }}><Tooltip title={cluster.title}><span>{`${cluster.DestinationUnid}/ep${cluster.DestinationEp}`}</span></Tooltip></td>
        <td style={{ color: cluster.color }}><Tooltip title={cluster.title}><span>{cluster.ClusterName}</span></Tooltip></td>
        <td>
          <span hidden={item.Node.ep[item.EndPoint].Clusters.Binding.SupportedCommands?.indexOf("Unbind") === -1}>
            <Tooltip title="Unbind">
              <span className="icon">
                <FiIcons.FiTrash2 color="#dc3545" onClick={() => this.sendCommand(item.Node.Unid, item.EndPoint, "Unbind",
                  {
                    ClusterName: cluster.ClusterName,
                    DestinationUnid: cluster.DestinationUnid,
                    DestinationEp: cluster.DestinationEp
                  }
                )} />
              </span>
            </Tooltip>
          </span>
        </td>
      </>
    );
  }

  sortArray = (array: any) => {
    if (!array) return [];
    array.sort((a: any, b: any) => {
      if (a.DestinationUnid > b.DestinationUnid) return 1;
      else if (a.DestinationUnid < b.DestinationUnid) return -1;
      else if (a.DestinationEp > b.DestinationEp) return 1;
      else if (a.DestinationEp < b.DestinationEp) return -1;
      else return 0;
    });
    return array;
  }

  toggleIsAllExpanded = () => {
    this.state.List.forEach((value: any, key: string) => {
      value.IsExpanded = !this.state.IsAllExpanded;
    });
    this.setState({ IsAllExpanded: !this.state.IsAllExpanded });
  }

  getEpList = () => {
    if (this.state.BindCmd.DestinationUnid)
      return Object.keys(this.props.NodeList.find((i: any) => i.Unid === this.state.BindCmd.DestinationUnid).ep) || []
    else return [];
  }

  getClusterList = () => {
    if (this.state.BindCmd.Unid && this.state.BindCmd.EndPoint)
      return this.props.NodeList.find((i: any) => i.Unid === this.state.BindCmd.Unid)?.ep[this.state.BindCmd.EndPoint]?.Clusters?.Binding?.Attributes?.BindableClusterList?.Reported || [];
    return [];
  }

  render() {
    return (
      <>
        <h3>Binding</h3>
        {(this.state.List.size === 0 || !this.props.IsConnected)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table>
            <thead>
              <tr>
                <th className="no-padding-h">
                  <span className='icon small' onClick={this.toggleIsAllExpanded}>
                    {this.state.IsAllExpanded ? <BsIcons.BsDashSquare /> : <BsIcons.BsPlusSquare />}
                  </span>
                </th>
                <th>Node</th>
                <th>Bindable Cluster List</th>
                <th>Table Full</th>
                <th>Destination</th>
                <th>Cluster Name</th>
                <th>&ensp;</th>
                <th>&ensp;</th>
              </tr>
            </thead>
            <tbody>
              {[...this.state.List.values()].map((item: any, index: number) => {
                let isOffline = item.Node.NetworkStatus === "Offline" || item.Node.NetworkStatus === "Unavailable";
                let commands = item.Node.ep[item.EndPoint].Clusters.Binding.SupportedCommands && item.Node.ep[item.EndPoint].Clusters.Binding.SupportedCommands.filter((cmd: any) => cmd !== "WriteAttributes");
                return (
                  <Fragment key={index}>
                    <Tooltip key={index} title={isOffline ? "Node is Offline" : ""}>
                      <tr className={isOffline ? "disabled" : ""}>
                        <td rowSpan={(item.IsExpanded && item.BindingTable?.length) || 1} className="no-padding-h">
                          <span className='icon small' onClick={() => { item.IsExpanded = !item.IsExpanded; }} hidden={!item.BindingTable?.length}>
                            {item.IsExpanded ? <BsIcons.BsDashSquare /> : <BsIcons.BsPlusSquare />}</span>
                        </td>
                        <td rowSpan={(item.IsExpanded && item.BindingTable?.length) || 1}>
                          <EditableAttribute Node={item.Node} EpName={item.EndPoint} Cluster={item.Node.ep[item.EndPoint].Clusters.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
                            SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={isOffline} />
                        </td>
                        <td rowSpan={(item.IsExpanded && item.BindingTable?.length) || 1}>
                          {item.BindableClusterList?.join(", ")}
                        </td>
                        <td rowSpan={(item.IsExpanded && item.BindingTable?.length) || 1}>
                          {item.BindingTableFull === true ? <FiIcons.FiCheck color="#28a745" /> : (item.BindingTableFull === false ? <FiIcons.FiXCircle color="#6c757d" /> : "")}
                        </td>
                        {item.BindingTable?.length && item.IsExpanded
                          ? this.getRow(item, item.BindingTable[0])
                          : <td colSpan={3} className="cluster-info text-center"><i><b>Binding Table Count:</b> {item.BindingTable?.length || 0}</i></td>
                        }
                        <td rowSpan={(item.IsExpanded && item.BindingTable?.length) || 1} className="padding-v-5">
                          {commands && commands.length
                            ? <DropdownButton variant="outline-primary" title="Commands" className="float-right" disabled={isOffline} size="sm">
                              <Dropdown.Item onClick={this.preBindCommand.bind(this, item)} hidden={commands.indexOf("Bind") === -1}>Bind</Dropdown.Item>
                              <Dropdown.Item onClick={this.preUnBindCommand.bind(this, item)} hidden={commands.indexOf("Unbind") === -1 || !item.BindingTable?.length}>Unbind</Dropdown.Item>
                              <Dropdown.Item onClick={this.preSendCommand.bind(this, item.Node.Unid, item.EndPoint, "BindToProtocolController")} hidden={commands.indexOf("BindToProtocolController") === -1}>Bind To Protocol Controller</Dropdown.Item>
                              <Dropdown.Item onClick={this.preSendCommand.bind(this, item.Node.Unid, item.EndPoint, "UnbindFromProtocolController")} hidden={commands.indexOf("UnbindFromProtocolController") === -1}>Unbind From Protocol Controller</Dropdown.Item>
                            </DropdownButton>
                            : <></>}
                        </td>
                      </tr>
                    </Tooltip>
                    {
                      item.BindingTable?.length && item.IsExpanded
                        ? item.BindingTable.map((cluster: any, clusterIndex: number) => {
                          if (clusterIndex === 0) return null;
                          return (
                            <tr key={`${index}-${clusterIndex}`} className={isOffline ? "disabled" : ""}>
                              {this.getRow(item, cluster)}
                            </tr>)
                        })
                        : null
                    }
                  </Fragment>
                )
              })}
            </tbody>
          </Table>
        }

        <CommandDlg ref={this.changeCommandDlg}
          SocketServer={this.props.SocketServer}
          ClusterType={"Binding"} />

        <Modal show={this.state.ShowBindModel} onHide={() => this.setState({ ShowBindModel: false })}>
          <Modal.Header>
            <Modal.Title>
              Fill Bind Command Fields
            </Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <div className='col-sm-12 margin-v-20'>
              <TextField size="small" fullWidth={true} name="DestinationUnid" select label="Destination Unid" value={this.state.BindCmd.DestinationUnid || ""} defaultValue={this.state.BindCmd.DestinationUnid || ""} onChange={this.handleChange} variant="outlined">
                {this.props.NodeList.map((option: any) => {
                  return <MenuItem key={option.Unid} value={option.Unid}>
                    {option.Unid}
                  </MenuItem>
                })}
              </TextField>
            </div>
            <div className='col-sm-12 margin-v-20'>
              <TextField size="small" fullWidth={true} name="DestinationEp" select label="Destination Ep" value={this.state.BindCmd.DestinationEp || ""} onChange={this.handleChange} variant="outlined">
                {this.getEpList().map((ep: any) => {
                  return <MenuItem key={ep} value={ep}>
                    {ep}
                  </MenuItem>
                })}
              </TextField>
            </div>
            <div className='col-sm-12 margin-v-20'>
              <TextField size="small" fullWidth={true} name="ClusterName" select label="Cluster Name" value={this.state.BindCmd.ClusterName || ""} onChange={this.handleChange} variant="outlined">
                {this.getClusterList().map((cluster: any) => {
                  return <MenuItem key={cluster} value={cluster}>
                    {cluster}
                  </MenuItem>
                })}
              </TextField>
            </div>
          </Modal.Body>
          <Modal.Footer>
            <Button variant="outline-primary" onClick={this.bindCommand} disabled={!this.state.BindCmd.ClusterName || !this.state.BindCmd.DestinationUnid || !this.state.BindCmd.DestinationEp}>OK</Button>
            <Button variant="primary" onClick={() => this.setState({ ShowBindModel: false })}>Cancel</Button>
          </Modal.Footer>
        </Modal >

        <Modal show={this.state.ShowUnBindModel} onHide={() => this.setState({ ShowUnBindModel: false })}>
          <Modal.Header>
            <Modal.Title>
              Choose Cluster to Unbind
            </Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <div className='col-sm-12 margin-v-20'>
              <TextField size="small" fullWidth={true} name="cluster-name" select label="Cluster Name" value={this.state.UnBindCluster} defaultValue={this.state.UnBindCluster} onChange={this.handleUnBindClusterChange} variant="outlined">
                {this.sortArray(this.unBindItem?.BindingTable).map((cluster: any) => {
                  return <MenuItem key={`[${cluster.DestinationUnid}-${cluster.DestinationEp}-${cluster.ClusterName}`} value={cluster}>
                    {`[${cluster.DestinationUnid}/ep${cluster.DestinationEp}] ${cluster.ClusterName}`}
                  </MenuItem>
                })}
              </TextField>
            </div>
          </Modal.Body>
          <Modal.Footer>
            <Button variant="outline-primary" onClick={this.unBindCommand} disabled={!this.state.UnBindCluster?.ClusterName}>OK</Button>
            <Button variant="primary" onClick={() => this.setState({ ShowUnBindModel: false })}>Cancel</Button>
          </Modal.Footer>
        </Modal >

      </>
    )
  };
}

export default Binding;