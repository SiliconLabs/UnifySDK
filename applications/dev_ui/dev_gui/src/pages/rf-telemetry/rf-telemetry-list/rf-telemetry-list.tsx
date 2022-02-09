import React from 'react';
import * as FiIcons from 'react-icons/fi';
import { Col, Row, Table } from 'react-bootstrap';
import * as CgIcons from 'react-icons/cg';
import Tooltip from '@material-ui/core/Tooltip';
import { Link } from 'react-router-dom';
import { RFTelemetryListProps } from './rf-telemetry-list-types';


export class RFTelemetryList extends React.Component<RFTelemetryListProps, {}> {


  toggle(unid: any, value: boolean) {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "update-cluster-attr",
        data: {
          Unid: unid,
          ClusterType: "ProtocolController/RFTelemetry",
          Payload: { TxReportEnabled: value }
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
        SuccessRate: `${Math.round(statistics.SuccessRate * 100)} %`,
        AverageTime: statistics.AverageTime ? `${Math.round(statistics.AverageTime)} ms` : "-",
        AverageRepeaters: statistics.AverageRepeaters ? Math.round(statistics.AverageRepeaters) : "-",
        IsSupportEnable: item.RFTelemetry.SupportedCommands?.filter((i: any) => i === "WriteAttributes").length,
        IsController: true
      });

      let destinations = (item.RFTelemetry.TxReport?.Destinations && Object.keys(item.RFTelemetry.TxReport?.Destinations)) || [];
      destinations.forEach((dst: any) => {
        let dstStatistics = item.RFTelemetry.TxReport.Destinations[dst].Statistics;
        list.push({
          Unid: item.Unid,
          Enabled: null,
          DestinationUNID: dst,
          Count: dstStatistics.Count || "-",
          SuccessRate: `${Math.round(dstStatistics.SuccessRate * 100)} %`,
          AverageTime: dstStatistics.AverageTime ? `${Math.round(dstStatistics.AverageTime)} ms` : "-",
          AverageRepeaters: dstStatistics.AverageRepeaters ? Math.round(dstStatistics.AverageRepeaters) : "-",
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
                <th>Enabled</th>
                <th>Destination UNID</th>
                <th>Transmission count</th>
                <th>Transmission Success rate (%)</th>
                <th>Average transmission time (ms)</th>
                <th>Average Number of repeaters</th>
                <th>&ensp;</th>
              </tr>
            </thead>
            <tbody>
              {list.map((item: any, index: number) => {
                return (
                  <tr key={index} className={item.SuccessRate && item.SuccessRate < 0.95 ? (item.SuccessRate > 0.80 ? "warning-background" : "error-background") : ""}>
                    <td>{item.IsController ? item.Unid : ""}</td>
                    <td>{item.Enabled === true ? <FiIcons.FiCheck color="#28a745" /> : (item.Enabled === false ? <FiIcons.FiXCircle color="#6c757d" /> : "")}</td>
                    <td>{item.DestinationUNID}</td>
                    <td>{item.Count}</td>
                    <td>{item.SuccessRate}</td>
                    <td>{item.AverageTime}</td>
                    <td>{item.AverageRepeaters}</td>
                    <td>
                      <Tooltip title="View Telemetry" className="float-right" hidden={item.IsController}>
                        <span className="icon">
                          <Link to={`/rftelemetry/${item.Unid}/${item.DestinationUNID}`}><CgIcons.CgEye className="margin-h-5" /></Link>
                        </span>
                      </Tooltip>
                      <Tooltip title={item.Enabled ? "Disable" : "Enable"} className="float-right" hidden={!item.IsSupportEnable}>
                        <span className="icon">
                          {item.Enabled ? <FiIcons.FiStopCircle color="#dc3545" className="margin-h-5" onClick={this.toggle.bind(this, item.Unid, false)} />
                            : <FiIcons.FiPlayCircle color="#28a745" className="margin-h-5" onClick={this.toggle.bind(this, item.Unid, true)} />}
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