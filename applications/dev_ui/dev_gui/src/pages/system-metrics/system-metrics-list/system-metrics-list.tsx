import React from 'react';
import { Button, Col, Modal, Row, Table } from 'react-bootstrap';
import * as CgIcons from 'react-icons/cg';
import { Link } from 'react-router-dom';
import { SystemMetricsListProps, SystemMetricsListState } from './system-metrics-list-types';
import { TextField, Tooltip } from '@mui/material';

export class SystemMetricsList extends React.Component<SystemMetricsListProps, SystemMetricsListState> {
  constructor(props: SystemMetricsListProps) {
    super(props);
    this.state = {
      ShowModal: false,
      CurrentMid: "",
      CurrentInterval: 0
    };
  }

  toggleModal(value: boolean) {
    this.setState({ ShowModal: value });
  }

  changeReportInterval(mid: string, interval: number) {
    this.setState({ ShowModal: true, CurrentMid: mid, CurrentInterval: interval });
  }

  setReportInterval = () => {
    this.props.SocketServer.send(JSON.stringify({
      type: "update-metric-interval",
      data: {
        Mid: this.state.CurrentMid,
        Payload: { ReportingIntervalSeconds: this.state.CurrentInterval }
      }
    }));
    this.setState({ ShowModal: false });
  }

  render() {
    return (
      <>
        <h3>System Metrics</h3>
        {(!this.props.SystemMetricsList || !this.props.SystemMetricsList)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table className='nodes-table'>
            <thead>
              <tr className="">
                <th>MID</th>
                <th><Tooltip title="Reporting Interval Seconds"><span>RI, s</span></Tooltip></th>
                <th>CPU Usage, %</th>
                <th>RAM Total, MB</th>
                <th>RAM Available, MB</th>
                <th><Tooltip title="Virtual Memory Total"><span>VM Used, MB</span></Tooltip></th>
                <th><Tooltip title="Virtual Memory Used"><span>VM Used, MB</span></Tooltip></th>
                <th>Disks Usage</th>
                <th>&ensp;</th>
              </tr>
            </thead>
            <tbody>
              {Object.keys(this.props.SystemMetricsList).map((key, index) => {
                let metric = this.props.SystemMetricsList[key];
                return (
                  <tr key={index}>
                    <td>{key}</td>
                    <td>{metric.Attributes?.ReportingIntervalSeconds}</td>
                    <td>{metric.Attributes?.CPUUsagePercent === undefined ? "" : metric.Attributes.CPUUsagePercent[metric.Attributes.CPUUsagePercent.length - 1]?.toFixed(2)}</td>
                    <td>{metric.Attributes?.RAMTotalMB?.toFixed(2)}</td>
                    <td>{metric.Attributes?.RAMAvailableMB?.toFixed(2)}</td>
                    <td>{metric.Attributes?.VirtualMemoryTotalMB?.toFixed(2)}</td>
                    <td>{metric.Attributes?.VirtualMemoryUsedMB?.toFixed(2)}</td>
                    <td>{metric.Attributes?.DisksUsage?.toFixed(2)}</td>
                    <td className="text-center" >
                      <span hidden={!metric.SupportedCommands || metric.SupportedCommands.indexOf("WriteAttributes") === -1}>
                        <Tooltip title="Set Interval" className="text-center">
                          <span className="icon">
                            <CgIcons.CgTimer onClick={() => this.changeReportInterval(key, metric.Attributes?.ReportingIntervalSeconds ?? 0)} />
                          </span>
                        </Tooltip>
                      </span>
                      <span hidden={!metric.Attributes} className="padding-l-5">
                        <Tooltip title="View System Metrics">
                          <span className="icon">
                            <Link to={`/systemmetrics/${key}`}><CgIcons.CgEye /></Link>
                          </span>
                        </Tooltip>
                      </span>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </Table>
        }

        <Modal show={this.state.ShowModal} onHide={() => this.toggleModal(false)} >
          <Modal.Header>
            <Modal.Title>
              Reporting Interval, seconds
            </Modal.Title>
          </Modal.Header>
          <Modal.Body>
            <TextField size="small" fullWidth={true} label="Reporting Interval" value={this.state.CurrentInterval} type="number"
              onChange={(e: any) => this.setState({ CurrentInterval: e.target.value })} variant="outlined" />
          </Modal.Body>
          <Modal.Footer>
            <Button variant="outline-primary" onClick={this.setReportInterval}>Ok</Button>
            <Button variant="primary" onClick={() => this.toggleModal(false)}>Cancel</Button>
          </Modal.Footer>
        </Modal>
      </>
    )
  };
}

export default SystemMetricsList;