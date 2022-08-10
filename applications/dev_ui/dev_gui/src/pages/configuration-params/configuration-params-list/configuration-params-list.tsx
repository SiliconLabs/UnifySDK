import React, { Fragment } from 'react';
import * as FiIcons from 'react-icons/fi';
import * as BsIcons from 'react-icons/bs';
import { Col, Dropdown, DropdownButton, Row, Table } from 'react-bootstrap';
import * as CgIcons from 'react-icons/cg';
import { Link } from 'react-router-dom';
import { ConfigurationParamsListProps, ConfigurationParamsListState } from './configuration-params-list-types';
import EditableAttribute from '../../../components/editable-attribute/editable-attribute';
import CommandDlg from '../../../components/command-dlg/command-dlg';
import { ClusterTypeAttrs } from '../../../cluster-types/cluster-type-attributes';
import { Tooltip } from '@mui/material';


export class ConfigurationParamsList extends React.Component<ConfigurationParamsListProps, ConfigurationParamsListState> {
  constructor(props: ConfigurationParamsListProps) {
    super(props);
    this.state = {
      List: this.getList(this.props.NodeList),
      IsAllExpanded: false
    }
    this.changeCommandDlg = React.createRef();
  }

  changeCommandDlg: any;

  preSendCommand(unid: string, endPoint: string, cmd: string) {
    let command = ClusterTypeAttrs["ConfigurationParameters"].server.commands.find((i: { name: string; }) => i.name === cmd);
    if (!command)
      return;

    if (command && command.fields && command.fields.length) {
      this.changeCommandDlg.current.updateState(`${unid}/${endPoint}`, command, true);
    } else
      this.props.SocketServer.send(JSON.stringify(
        {
          type: "run-cluster-command",
          data: {
            Unid: `${unid}/${endPoint}`,
            ClusterType: "ConfigurationParameters",
            Cmd: command.name,
            Payload: {}
          }
        }));
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
        if (!item.ep[ep].Clusters?.ConfigurationParameters)
          return;
        list.set(item.Unid + ep, {
          Node: item,
          EndPoint: ep,
          Params: item.ep[ep].Clusters?.ConfigurationParameters?.Attributes?.ConfigurationParameters?.Reported,
          IsExpanded: (this.state && this.state.List && this.state.List.get(item.Unid + ep)?.IsExpanded) || false
        });
      });
    });
    return list;
  }

  getParamRow = (node: any, param: any) => {
    return (
      <>
        <td>{param.ParameterId}</td>
        <td>{param.Name}</td>
        <td>{param.Value}</td>
        <td>{param.MinimumValue}</td>
        <td>{param.DefaultValue}</td>
        <td>{param.MaximumValue}</td>
        <td>{param.ReadOnly === true ? <FiIcons.FiCheck color="#28a745" /> : (param.ReadOnly === false ? <FiIcons.FiXCircle color="#6c757d" /> : "")}</td>
        <td>
          <Tooltip title="View Parameter" className="float-right">
            <span className="icon">
              <Link to={`/configurationparameters/${node.Node.Unid}/${node.EndPoint}/${param.ParameterId}`}><CgIcons.CgEye className="margin-h-5" /></Link>
            </span>
          </Tooltip>
        </td>
      </>
    );
  }

  toggleIsAllExpanded = () => {
    this.state.List.forEach((value: any, key: string) => {
      value.IsExpanded = !this.state.IsAllExpanded;
    });
    this.setState({ IsAllExpanded: !this.state.IsAllExpanded });
  }

  render() {
    return (
      <>
        <h3>Configuration Parameters</h3>
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
                <th>Parameter Id</th>
                <th>Name</th>
                <th>Value</th>
                <th>Min</th>
                <th>Default</th>
                <th>Max</th>
                <th>Read-only</th>
                <th>&ensp;</th>
                <th>&ensp;</th>
              </tr>
            </thead>
            <tbody>
              {[...this.state.List.values()].map((item: any, index: number) => {
                let isOffline = item.Node.NetworkStatus === "Offline" || item.Node.NetworkStatus === "Unavailable";
                let commands = item.Node.ep[item.EndPoint].Clusters.ConfigurationParameters.SupportedCommands && item.Node.ep[item.EndPoint].Clusters.ConfigurationParameters.SupportedCommands.filter((cmd: any) => cmd !== "WriteAttributes");
                return (
                  <Fragment key={index}>
                    <Tooltip key={index} title={isOffline ? "Node is Offline" : ""}>
                      <tr className={isOffline ? "disabled" : ""}>
                        <td rowSpan={(item.IsExpanded && item.Params?.length) || 1} className="no-padding-h">
                          <span className='icon small' onClick={() => { item.IsExpanded = !item.IsExpanded; }} hidden={!item.Params?.length}>
                            {item.IsExpanded ? <BsIcons.BsDashSquare /> : <BsIcons.BsPlusSquare />}</span>
                        </td>
                        <td rowSpan={(item.IsExpanded && item.Params?.length) || 1}>
                          <EditableAttribute Node={item.Node} EpName={item.EndPoint} Cluster={item.Node.ep[item.EndPoint].Clusters.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
                            SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={isOffline} />
                        </td>
                        {item.Params?.length && item.IsExpanded
                          ? this.getParamRow(item, item.Params[0])
                          : <td colSpan={8} className="cluster-info text-center"><i><b>Parameters Count:</b> {item.Params?.length || 0}</i></td>
                        }
                        <td rowSpan={(item.IsExpanded && item.Params?.length) || 1} className="padding-v-5">
                          {commands && commands.length
                            ? <DropdownButton variant="outline-primary" title="Commands" className="float-right" disabled={isOffline} size="sm">
                              {commands.map((cmd: string, cmdIndex: number) => {
                                return (
                                  <Dropdown.Item key={cmdIndex} onClick={this.preSendCommand.bind(this, item.Node.Unid, item.EndPoint, cmd)}> {cmd}</Dropdown.Item>
                                )
                              })
                              }
                            </DropdownButton>
                            : <></>}
                        </td>
                      </tr>
                    </Tooltip>
                    {item.Params?.length && item.IsExpanded
                      ? item.Params.map((param: any, indexParam: number) => {
                        if (indexParam === 0) return null;
                        return (
                          <tr key={indexParam} className={isOffline ? "disabled" : ""}>
                            {this.getParamRow(item, param)}
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
          ClusterType={"ConfigurationParameters"} />
      </>
    )
  };
}

export default ConfigurationParamsList;