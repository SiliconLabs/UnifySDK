import { Tooltip } from '@mui/material';
import React from 'react';
import { Col, Form, Row, Table } from 'react-bootstrap';
import * as CgIcons from 'react-icons/cg';
import { Link } from 'react-router-dom';
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
    let list: any = [];
    this.props.NodeList.filter((i: any) => i.RFTelemetry).forEach((item: any) => {
      let statistics = item.RFTelemetry.TxReport.Statistics;
      list.push({
        Unid: item.Unid,
        Enabled: item.RFTelemetry.Attributes?.TxReportEnabled?.Reported,
        DestinationUNID: "All",
        Count: statistics.Count || "-",
        SuccessRate: (statistics.SuccessRate * 100).toFixed(2) || 100,
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
          SuccessRate: (dstStatistics.SuccessRate * 100).toFixed(2),
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
                  <tr key={index} className={item.SuccessRate && item.SuccessRate < 95 ? (item.SuccessRate > 80 ? "warning-background" : "error-background") : ""}>
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
        }
      </>
    )
  };
}

export default RFTelemetryList;