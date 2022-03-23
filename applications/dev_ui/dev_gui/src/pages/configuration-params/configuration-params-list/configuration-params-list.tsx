import React, { Fragment } from 'react';
import * as FiIcons from 'react-icons/fi';
import { Button, Col, Row, Table } from 'react-bootstrap';
import * as CgIcons from 'react-icons/cg';
import Tooltip from '@material-ui/core/Tooltip';
import { Link } from 'react-router-dom';
import { ConfigurationParamsListProps } from './configuration-params-list-types';
import EditableAttribute from '../../../components/editable-attribute/editable-attribute';
import CommandDlg from '../../../components/command-dlg/command-dlg';
import { ClusterTypeAttrs } from '../../../cluster-types/cluster-type-attributes';


export class ConfigurationParamsList extends React.Component<ConfigurationParamsListProps, {}> {
  constructor(props: ConfigurationParamsListProps) {
    super(props);
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

  render() {
    let list: any = [];
    this.props.NodeList.forEach((item: any) => {
      if (!item.ep)
        return;
      Object.keys(item.ep).forEach(ep => {
        if (!item.ep[ep].Clusters?.ConfigurationParameters)
          return;
        list.push({
          Node: item,
          EndPoint: ep,
          Params: item.ep[ep].Clusters?.ConfigurationParameters?.Attributes?.ConfigurationParameters?.Reported
        });
      });
    });
    return (
      <>
        <h3>Configuration Parameters</h3>
        {(!list || !list.length || !this.props.IsConnected)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table>
            <thead>
              <tr>
                <th>Node</th>
                <th>Parameter Id</th>
                <th>Name</th>
                <th>Value</th>
                <th>Min</th>
                <th>Default</th>
                <th>Max</th>
                <th>Read-only</th>
                <th>&ensp;</th>
                <th className='wd-500'>&ensp;</th>
              </tr>
            </thead>
            <tbody>
              {list.map((item: any, index: number) => {
                let isOffline = item.Node.NetworkStatus === "Offline" || item.Node.NetworkStatus === "Unavailable";
                return (
                  <Fragment key={index}>
                    <Tooltip key={index} title={isOffline ? "Node is Offline" : ""}>
                      <tr className={isOffline ? "disabled" : ""}>
                        <td rowSpan={item.Params?.length || 1} className="vertical-middle">
                          <EditableAttribute Node={item.Node} EpName={item.EndPoint} Cluster={item.Node.ep[item.EndPoint].Clusters.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
                            SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={isOffline} />
                        </td>
                        {item.Params?.length
                          ? this.getParamRow(item, item.Params[0])
                          : <td colSpan={7}>&ensp;</td>
                        }
                        <td rowSpan={item.Params?.length || 1} className="vertical-middle">
                          {item.Node.ep[item.EndPoint].Clusters.ConfigurationParameters.SupportedCommands && item.Node.ep[item.EndPoint].Clusters.ConfigurationParameters.SupportedCommands.filter((cmd: any) => cmd !== "WriteAttributes").map((cmd: string, index: number) => {
                            return <Button disabled={isOffline} key={index} className="margin-5" size="sm" onClick={this.preSendCommand.bind(this, item.Node.Unid, item.EndPoint, cmd)}>{cmd}</Button>
                          })}
                        </td>
                      </tr>
                    </Tooltip>
                    {item.Params?.length
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