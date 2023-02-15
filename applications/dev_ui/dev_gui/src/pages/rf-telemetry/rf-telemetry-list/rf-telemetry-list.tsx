import { Tooltip } from '@mui/material';
import React from 'react';
import { Card, Col, Form, Row, Table } from 'react-bootstrap';
import * as CgIcons from 'react-icons/cg';
import { Link } from 'react-router-dom';
import EditableAttribute from '../../../components/editable-attribute/editable-attribute';
import { RFTelemetryListProps } from './rf-telemetry-list-types';


export class RFTelemetryList extends React.Component<RFTelemetryListProps, {}> {

  toggle(unid: any, prop: string, event: any) {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "update-cluster-attr",
        data: {
          Unid: unid,
          ClusterType: "ProtocolController/RFTelemetry",
          Payload: { [prop]: event.target.checked }
        }
      }));
  }

  render() {
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    let list: any = [];
    this.props.NodeList.filter((i: any) => i.RFTelemetry).forEach((item: any) => {
      let statistics = item.RFTelemetry.TxReport.Statistics;
      list.push({
        Unid: item.Unid,
        Enabled: item.RFTelemetry.Attributes?.TxReportEnabled?.Reported,
        DestinationUNID: "All",
        Count: statistics.Count || "-",
        SuccessRate: statistics.SuccessRate !== null ? (statistics.SuccessRate * 100).toFixed(2) : null,
        AverageTime: statistics.AverageTime ? `${Math.round(statistics.AverageTime)} ms` : "-",
        AverageRepeaters: statistics.AverageRepeaters ? statistics.AverageRepeaters.toFixed(2) : "0.00",
        IsSupportEnable: item.RFTelemetry.SupportedCommands?.filter((i: any) => i === "WriteAttributes").length,
        IsController: true,
        IsPTIEnabled: item.RFTelemetry.Attributes?.PTIEnabled?.Reported,
      });

      let destinations = (item.RFTelemetry.TxReport?.Destinations && Object.keys(item.RFTelemetry.TxReport?.Destinations)) || [];
      destinations.forEach((dst: any) => {
        let dstStatistics = item.RFTelemetry.TxReport.Destinations[dst].Statistics;
        list.push({
          Unid: item.Unid,
          Enabled: null,
          DestinationUNID: dst,
          Count: dstStatistics.Count || "-",
          SuccessRate: dstStatistics.SuccessRate !== null ? (dstStatistics.SuccessRate * 100).toFixed(2) : null,
          AverageTime: dstStatistics.AverageTime ? `${Math.round(dstStatistics.AverageTime)} ms` : "-",
          AverageRepeaters: dstStatistics.AverageRepeaters ? dstStatistics.AverageRepeaters.toFixed(2) : "0.00",
          IsSupportEnable: false,
          IsController: false
        });
      });
    });
    return (
      <>
        <h3>The RF Telemetry List</h3>
        {(!list || !list.length || !this.props.IsConnected)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : (isMobile
            ? <div className='table-content rf-telemetry'>
              {
                list.map((item: any, index: number) => {
                  return (
                    <Card key={index} className={item.SuccessRate !== null && item.SuccessRate < 95 ? (item.SuccessRate > 80 ? "warning-background inline margin-v-10" : "error-background inline margin-v-10") : "inline margin-v-10"}>
                      <Card.Header className='flex'>
                        <div className="col-sm-6">
                          <b>{item.IsController ? item.Unid : ""}</b>
                        </div>
                        <div className="col-sm-6 float-right">
                          <span className="icon float-right" hidden={item.IsController}>
                            <Link to={`/rftelemetry/${item.Unid}/${item.DestinationUNID}`}><CgIcons.CgEye /></Link>
                          </span>
                        </div>
                      </Card.Header>
                      <Card.Body>
                        <div className='col-sm-12'>
                          <div className="col-sm-6 inline" hidden={!item.IsController || item.Enabled === undefined}><b><i>On/Off RF Telemetry: </i></b><Form.Check  className="inline" id={`rf-switch-${index}`} disabled={!item.IsSupportEnable} checked={item.Enabled} onChange={this.toggle.bind(this, item.Unid, "TxReportEnabled")} /></div>
                          <div className="col-sm-6 inline" hidden={!item.IsController || item.IsPTIEnabled === undefined}><b><i>On/Off PTI: </i></b><Form.Check className="inline" id={`pti-switch-${index}`} disabled={!item.IsSupportEnable} checked={item.IsPTIEnabled} onChange={this.toggle.bind(this, item.Unid, "PTIEnabled")} /></div>
                          <div className="col-sm-6 inline"><b><i>Destination UNID: </i></b>{item.DestinationUNID}</div>
                          <div className="col-sm-6 inline"><b><i>Transmission count: </i></b>{item.Count}</div>
                          <div className="col-sm-12 inline"><b><i>Transmission Success rate (%): </i></b>{item.SuccessRate} %</div>
                          <div className="col-sm-12 inline"><b><i>Average transmission time (ms): </i></b>{item.AverageTime}</div>
                          <div className="col-sm-12 inline"><b><i>Average Number of repeaters: </i></b>{item.AverageRepeaters}</div>
                        </div>
                      </Card.Body>
                    </Card>
                  );
                })
              }
            </div>
            : <Table striped hover>
              <thead>
                <tr>
                  <th>Unid</th>
                  <th>Destination UNID</th>
                  <th>Transmission count</th>
                  <th>Transmission Success rate (%)</th>
                  <th>Average transmission time (ms)</th>
                  <th>Average Number of repeaters</th>
                  <th>PTI</th>
                  <th>RF Telemetry</th>
                </tr>
              </thead>
              <tbody>
                {list.map((item: any, index: number) => {
                  return (
                    <tr key={index} className={item.SuccessRate !== null && item.SuccessRate < 95 ? (item.SuccessRate > 80 ? "warning-background" : "error-background") : ""}>
                      <td>{item.IsController ? item.Unid : ""}</td>
                      <td>{item.DestinationUNID}</td>
                      <td>{item.Count}</td>
                      <td>{item.SuccessRate} %</td>
                      <td>{item.AverageTime}</td>
                      <td>{item.AverageRepeaters}</td>
                      <td>
                        <span hidden={!item.IsController || item.IsPTIEnabled === undefined} className="pointer">
                          <Tooltip title="On/Off PTI" className="text-center">
                            <Form.Check type="switch" id={`pti-switch-${index}`} disabled={!item.IsSupportEnable} checked={item.IsPTIEnabled} onChange={this.toggle.bind(this, item.Unid, "PTIEnabled")} />
                          </Tooltip>
                        </span>
                      </td>
                      <td className="text-center" >
                        <span hidden={!item.IsController || item.Enabled === undefined}>
                          <Tooltip title="On/Off RF Telemetry" className="text-center">
                            <span className="pointer">
                              <Form.Check type="switch" id={`rf-switch-${index}`} disabled={!item.IsSupportEnable} checked={item.Enabled} onChange={this.toggle.bind(this, item.Unid, "TxReportEnabled")} />
                            </span>
                          </Tooltip>
                        </span>
                        <Tooltip title="View Telemetry" hidden={item.IsController}>
                          <span className="icon">
                            <Link to={`/rftelemetry/${item.Unid}/${item.DestinationUNID}`}><CgIcons.CgEye /></Link>
                          </span>
                        </Tooltip>
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </Table>
          )
        }
      </>
    )
  };
}

export default RFTelemetryList;