import React from 'react';
import { Card, Col, ProgressBar, Row, Tab, Table, Tabs } from 'react-bootstrap';
import { OTAProps, OTAState, Image } from './ota-types';
import './ota.css';
import ClusterTypeTooltip from '../../components/cluster-type-tooltip/cluster-type-tooltip';
import { Tooltip } from '@mui/material';

export class OTA extends React.Component<OTAProps, OTAState> {
  constructor(props: OTAProps) {
    super(props);
    this.state = {
      OTAImageList: [],
      NodeList: this.props.NodeList
    };

  }

  updateImageList(imageList: Image[]) {
    this.setState({ OTAImageList: imageList });
  }

  updateNodeList(nodeList: any[]) {
    this.setState({ NodeList: nodeList });
  }

  render() {
    let list: any[] = [];
    this.state.NodeList.forEach(node => {
      if (!node.ep) return;
      return Object.keys(node.ep).forEach((ep) => {
        if (!node.ep[ep].OTA || !node.ep[ep].OTA.Attributes || !node.ep[ep].OTA.Attributes.UIID) return;
        return Object.keys(node.ep[ep].OTA.Attributes.UIID).forEach((uiid, index) => {
          let itemAttr = node.ep[ep].OTA.Attributes.UIID[uiid];
          if (!itemAttr) return;
          let progress = 0;
          if (itemAttr.Size?.Reported !== undefined && itemAttr.Offset?.Reported !== undefined)
            progress = Math.round((itemAttr.Offset.Reported / itemAttr.Size.Reported) * 100);
          list.push({
            Unid: node.Unid,
            Uiid: uiid,
            EpData: { [ep]: node.ep[ep] },
            CurrentVersion: itemAttr.CurrentVersion?.Reported,
            TargetVersion: itemAttr.TargetVersion?.Reported,
            ApplyAfter: itemAttr.ApplyAfter?.Reported,
            Status: itemAttr.Status?.Reported,
            LastError: itemAttr.LastError?.Reported,
            Progress: progress
          });
        });
      })
    })
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    return (
      <>
        <Tabs hidden={!this.props.IsConnected} defaultActiveKey="updates">
          <Tab eventKey="updates" title="Available updates" className="ota-content">
            {(!list.length)
              ? <Row>
                <Col xs={12} className="text-center">
                  <span className="no-content">No Content</span>
                </Col>
              </Row>
              : (isMobile
                ? <div className='table-content'>
                  {
                    list.map((item, index) => {
                      return (
                        <Card key={index} className="inline margin-v-10">
                          <Card.Header className='flex'>
                            <div className="col-sm-8">
                              <i>UNID:</i> <b>{item.Unid}</b>
                            </div>
                          </Card.Header>
                          <Card.Body>
                            <div className='col-sm-12'>
                              <div className="col-sm-12 inline"><b><i>UIID: </i></b>{item.Uiid}</div>
                              <div className="col-sm-12 flex"><b><i>Type: </i></b><ClusterTypeTooltip Ep={item.EpData} /></div>
                              <div className="col-sm-6 inline"><b><i>Current Version: </i></b>{item.CurrentVersion}</div>
                              <div className="col-sm-6 inline"><b><i>Target Version: </i></b>{item.TargetVersion}</div>
                              <div className="col-sm-6 inline"><b><i>Apply after: </i></b>{item.ApplyAfter}</div>
                              <div className="col-sm-6 inline"><b><i>Last Error: </i></b>{item.LastError}</div>
                              <div className="col-sm-12 inline"><b><i>Status: </i></b><span>{item.Status}</span>
                                <span hidden={!item.Progress}>
                                  <Tooltip title={`${item.Progress}%`}>
                                    <ProgressBar now={item.Progress} label={`${item.Progress}%`} />
                                  </Tooltip>
                                </span>
                              </div>
                            </div>
                          </Card.Body>
                        </Card>
                      );
                    })}
                </div>
                : <Table striped hover>
                  <thead>
                    <tr className="">
                      <th>UNID</th>
                      <th>UIID</th>
                      <th>Type</th>
                      <th>Current Version</th>
                      <th>Target Version</th>
                      <th>Apply after</th>
                      <th>Last Error</th>
                      <th>Status</th>
                    </tr>
                  </thead>
                  <tbody>
                    {list.map((item, index) => {
                      return (
                        <tr key={index}>
                          <td>{item.Unid}</td>
                          <td>{item.Uiid}</td>
                          <td className="flex"><ClusterTypeTooltip Ep={item.EpData} /></td>
                          <td>{item.CurrentVersion}</td>
                          <td>{item.TargetVersion}</td>
                          <td>{item.ApplyAfter}</td>
                          <td>{item.LastError}</td>
                          <td><span>{item.Status}</span>
                            <span hidden={!item.Progress}>
                              <Tooltip title={`${item.Progress}%`}>
                                <ProgressBar now={item.Progress} label={`${item.Progress}%`} />
                              </Tooltip>
                            </span>
                          </td>
                        </tr>
                      );
                    })}
                  </tbody>
                </Table>
              )
            }
          </Tab>
          <Tab eventKey="images" title="Images">
            {(!this.state.OTAImageList || !this.state.OTAImageList.length)
              ? <Row>
                <Col xs={12} className="text-center">
                  <span className="no-content">No Content</span>
                </Col>
              </Row>
              : (isMobile
                ? <div className='table-content'>
                   {this.state.OTAImageList.map((item, index) => {
                      return (
                        <Card key={index} className="inline margin-v-10">
                          <Card.Header className='flex'>
                            <div className="col-sm-8">
                              <i>UIID:</i> <b>{item.Uiid}</b>
                            </div>
                          </Card.Header>
                          <Card.Body>
                            <div className='col-sm-12'>
                              <div className="col-sm-12 inline"><b><i>UNID(s): </i></b>{item.Unid?.length ? item.Unid.join(", ") : "all"}</div>
                              <div className="col-sm-12 inline"><b><i>Filename: </i></b>{item.Filename}</div>
                              <div className="col-sm-6 inline"><b><i>Version: </i></b>{item.Version}</div>
                              <div className="col-sm-6 inline"><b><i>Apply after: </i></b>{item.ApplyAfter}</div>
                            </div>
                          </Card.Body>
                        </Card>
                      );
                    })}
                </div>
                : <Table striped hover>
                  <thead>
                    <tr className="">
                      <th>UIID</th>
                      <th>Filename</th>
                      <th>Version</th>
                      <th>UNID(s)</th>
                      <th>Apply after</th>
                    </tr>
                  </thead>
                  <tbody>
                    {this.state.OTAImageList.map((item, index) => {
                      return (
                        <tr key={index}>
                          <td>{item.Uiid}</td>
                          <td>{item.Filename}</td>
                          <td>{item.Version}</td>
                          <td>{item.Unid.length ? item.Unid.join(", ") : "all"}</td>
                          <td>{item.ApplyAfter}</td>
                        </tr>
                      );
                    })}
                  </tbody>
                </Table>
              )
            }
          </Tab>
        </Tabs>
      </>
    )
  };
}

export default OTA;