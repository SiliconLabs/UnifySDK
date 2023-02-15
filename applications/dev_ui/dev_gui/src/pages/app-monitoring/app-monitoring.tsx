import React from 'react';
import * as FiIcons from 'react-icons/fi';
import { Card, Col, Row, Table } from 'react-bootstrap';
import { AppMonitoringProps } from './app-monitoring-types';
import { Tooltip } from '@mui/material';


export class AppMonitoring extends React.Component<AppMonitoringProps, {}> {
  constructor(props: AppMonitoringProps) {
    super(props);
  }

  render() {
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    return (
      <>
        <h3>Application Monitoring</h3>
        {(!this.props.AppMonitoringList || !Object.keys(this.props.AppMonitoringList).length)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : (isMobile
            ? <div className='table-content'>
              {Object.keys(this.props.AppMonitoringList).map((item: any, index: number) => {
                let className = this.props.AppMonitoringList[item].ApplicationConnected ? "" : "disabled";
                return (
                  <Card key={index} className="inline margin-v-10">
                    <Card.Header className='flex'>
                      <div className={`col-sm-6 ${className}`}>
                        <b>{item}</b>
                      </div>
                    </Card.Header>
                    <Card.Body className={className}>
                      <div className='col-sm-12'>
                        <div className="col-sm-6 inline"><b><i>Version </i></b>{this.props.AppMonitoringList[item].ApplicationVersion}</div>
                        <div className="col-sm-6 inline"><b><i>Connected: </i></b>
                        {this.props.AppMonitoringList[item].ApplicationConnected?.Reported === true
                        ? <Tooltip title="Application Connected"><span><FiIcons.FiCheck color="#28a745" /></span></Tooltip>
                        : (this.props.AppMonitoringList[item].ApplicationConnected?.Reported === false && this.props.AppMonitoringList[item].ApplicationConnected?.Desired === false
                          ? <Tooltip title="Application Shutdown"><span><FiIcons.FiXCircle color="#6c757d" /></span></Tooltip>
                          : (!this.props.AppMonitoringList[item].ApplicationConnected?.Reported && this.props.AppMonitoringList[item].ApplicationConnected?.Desired === true
                            ? <Tooltip title="Application unexpectedly disconnected"><span><FiIcons.FiAlertTriangle color="orange" /></span></Tooltip>
                            : ""))}</div>
                      </div>
                    </Card.Body>
                  </Card>
                );
              })}
            </div>
            : <Table>
              <thead>
                <tr>
                  <th>MQTT Client ID</th>
                  <th>Name</th>
                  <th>Connected</th>
                  <th>Version</th>
                </tr>
              </thead>
              <tbody>
                {Object.keys(this.props.AppMonitoringList).map((item: any, index: number) => {
                  let className = this.props.AppMonitoringList[item].ApplicationConnected ? "" : "disabled";
                  return (
                    <tr className={className} key={index}>
                      <td>{item}</td>
                      <td>{this.props.AppMonitoringList[item].ApplicationName?.Reported ?? "-"}</td>
                      <td>{this.props.AppMonitoringList[item].ApplicationConnected?.Reported === true
                        ? <Tooltip title="Application Connected"><span><FiIcons.FiCheck color="#28a745" /></span></Tooltip>
                        : (this.props.AppMonitoringList[item].ApplicationConnected?.Reported === false && this.props.AppMonitoringList[item].ApplicationConnected?.Desired === false
                          ? <Tooltip title="Application Shutdown"><span><FiIcons.FiXCircle color="#6c757d" /></span></Tooltip>
                          : (!this.props.AppMonitoringList[item].ApplicationConnected?.Reported && this.props.AppMonitoringList[item].ApplicationConnected?.Desired === true
                            ? <Tooltip title="Application unexpectedly disconnected"><span><FiIcons.FiAlertTriangle color="orange" /></span></Tooltip>
                            : ""))}</td>
                      <td>{this.props.AppMonitoringList[item].ApplicationVersion?.Reported ?? "-"}</td>
                    </tr>
                  )
                })}
              </tbody>
            </Table>
          )
        }
      </>
    )
  };
}

export default AppMonitoring;