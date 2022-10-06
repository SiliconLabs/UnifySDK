import React from 'react';
import { Button, Card, Col, Row, Tab, Table, Tabs } from 'react-bootstrap';
import { SceneItem, SceneListProps, SceneListState } from './scene-list-types';
import { Tooltip } from '@mui/material';
import * as FiIcons from 'react-icons/fi'
import ConfirmDlg from '../../../components/confirm-dlg/confirm-dlg';
import { Link } from 'react-router-dom';
import BaseClusters from '../../base-clusters/base-clusters';
import { ClusterViewOverrides } from '../../base-clusters/cluster-view-overrides';
import { ClusterTypeAttrs } from '../../../cluster-types/cluster-type-attributes';
import { ClusterTypes } from '../../../cluster-types/cluster-types';

export class SceneList extends React.Component<SceneListProps, SceneListState> {
  constructor(props: SceneListProps) {
    super(props);
    this.state = {
      ProcessingScene: {} as SceneItem,
      ActivePage: this.props.Storage.Get()?.ActiveScenePage || "Scenes"
    }
    this.changeConfirmDlg = React.createRef();
    this.changeClusters = React.createRef();
  }
  changeConfirmDlg: any;
  changeClusters: any;

  remove(item: any) {
    this.setState({ ProcessingScene: item }, () => {
      this.changeConfirmDlg?.current.update(
        `Remove Scene [GroupID:${this.state.ProcessingScene.GroupID}][SceneID:${this.state.ProcessingScene.SceneID}]`,
        `Are you sure, you want to remove this scene?`
      );
    });
  }

  confirmRemove = () => {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "save-scene",
        data: {
          GroupID: this.state.ProcessingScene.GroupID,
          SceneID: this.state.ProcessingScene.SceneID,
          Payload: null
        }
      }))
  }

  send = (scene: SceneItem, toAll?: boolean) => {
    if (toAll) {
      this.props.GroupList?.forEach((i: any) => {
        if (this.props.SceneList && this.props.SceneList[i.GroupId] && this.props.SceneList[i.GroupId][scene.SceneID as number])
          this.sendMessage(this.props.SceneList[i.GroupId][scene.SceneID as number]);
      });
    }
    else
      this.sendMessage(scene);
  }

  sendMessage = (scene: SceneItem) => {
    if (!scene?.SceneTableExtensions)
      return;
    Object.keys(scene.SceneTableExtensions).forEach(cluster => {
      Object.keys(scene.SceneTableExtensions[cluster])?.forEach(cmd => {
        this.props.SocketServer.send(JSON.stringify(
          {
            type: "run-group-command",
            data: {
              GroupId: scene.GroupID,
              ClusterType: cluster,
              Cmd: cmd,
              Payload: scene.SceneTableExtensions[cluster][cmd]
            }
          }))
      });
    });
  }

  render() {
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    return (
      <>
        <h3>Scene List</h3>
        <Tabs hidden={!this.props.IsConnected} defaultActiveKey={this.state.ActivePage} onSelect={(k: string | null) => { this.props.Storage.Set("ActiveScenePage", k); this.setState({ ActivePage: k || "" }); }}>
          <Tab eventKey="Scenes" title="Scenes">
            <br />
            <BaseClusters
              ref={this.changeClusters}
              SocketServer={this.props.SocketServer}
              IsConnected={this.props.IsConnected}
              ClusterType={(ClusterTypes as any)["Scenes"]}
              ClusterTypeAttrs={ClusterTypeAttrs["Scenes"]}
              NodeList={this.props.NodeList}
              GroupList={this.props.GroupList}
              ClusterViewOverrides={(ClusterViewOverrides as any)["Scenes"]}
              ShowTitle={false} />
          </Tab>
          <Tab eventKey="Configuration" title="Scenes Configuration">
            <br />
            <Row hidden={!this.props.IsConnected} >
              <Col xs={12}>
                <Link to={"/scenes/new/new"} >
                  <Button variant="outline-primary" className="float-left margin-b-10">Create Scene</Button>
                </Link>
              </Col>
            </Row>
            {(!this.props.SceneList || !Object.keys(this.props.SceneList).length)
              ? <Row>
                <Col xs={12} className="text-center">
                  <span className="no-content">No Content</span>
                </Col>
              </Row>
              :
              (isMobile
                ? <div className='table-content'>
                  {Object.keys(this.props.SceneList).map((groupId, groupIndex) => {
                    let groupScenes = Object.keys(this.props.SceneList[groupId]);
                    return groupScenes?.map((sceneId, sceneIndex) => {
                      let scene: any = this.props.SceneList[groupId][sceneId];
                      return (
                        <Card key={`${groupIndex}-${sceneIndex}`} className="inline margin-v-10">
                          <Card.Header className='flex'>
                            <div className="col-sm-7">
                              <b> <i>Group:</i></b> {`[ID: ${groupId}] ${this.props.GroupList.find(i => i.GroupId === scene.GroupID)?.GroupName ?? ""}`}
                            </div>
                            <div className='inline col-sm-5'>
                              <Tooltip title="Remove" className='float-right'>
                                <span className="icon">
                                  <FiIcons.FiTrash2 color="#dc3545" onClick={() => this.remove(scene)} />
                                </span>
                              </Tooltip>
                              <Tooltip title="Send to all Groups" className='float-right'>
                                <span className="icon">
                                  <FiIcons.FiSend className="margin-r-5" color="grey" onClick={() => this.send(scene, true)} />
                                </span>
                              </Tooltip>
                              <Tooltip title="Send" className='float-right'>
                                <span className="icon">
                                  <FiIcons.FiSend className="margin-r-5" onClick={() => this.send(scene)} />
                                </span>
                              </Tooltip>
                              <Tooltip title="View/Edit Scene" className='float-right margin-r-5'>
                                <span className="icon">
                                  <Link to={`/scenes/${groupId}/${sceneId}`}><FiIcons.FiEdit /></Link>
                                </span>
                              </Tooltip>
                            </div>
                          </Card.Header>
                          <Card.Body>
                            <div className='col-sm-12'>
                              <div className="col-sm-12 inline"><b><i>Scene: </i></b>{`[ID: ${sceneId}] ${scene.SceneName ?? ""}`}</div>
                              <div className="col-sm-12 inline"><b><i>Involved Clusters: </i></b>{Object.keys(scene.SceneTableExtensions).join(", ")}</div>
                            </div>
                          </Card.Body>
                        </Card>
                      );
                    })
                  })}
                </div>
                : <Table striped hover>
                  <thead>
                    <tr className="">
                      <th>Group</th>
                      <th>Scene</th>
                      <th>Involved Clusters</th>
                      <th className="wd-col-2">&ensp;</th>
                    </tr>
                  </thead>
                  <tbody>
                    {Object.keys(this.props.SceneList).map((groupId, groupIndex) => {
                      let groupScenes = Object.keys(this.props.SceneList[groupId]);
                      return groupScenes?.map((sceneId, sceneIndex) => {
                        let scene: any = this.props.SceneList[groupId][sceneId];
                        return (
                          <tr key={`${groupIndex}-${sceneIndex}`}>
                            <td rowSpan={groupScenes.length} hidden={sceneIndex > 0}>{`[ID: ${groupId}] ${this.props.GroupList.find(i => i.GroupId === scene.GroupID)?.GroupName ?? ""}`}</td>
                            <td>{`[ID: ${sceneId}] ${scene.SceneName ?? ""}`}</td>
                            <td>{Object.keys(scene.SceneTableExtensions).join(", ")}</td>
                            <td>
                              <Tooltip title="Remove" className='float-right'>
                                <span className="icon">
                                  <FiIcons.FiTrash2 color="#dc3545" onClick={() => this.remove(scene)} />
                                </span>
                              </Tooltip>
                              <Tooltip title="Send to all Groups" className='float-right'>
                                <span className="icon">
                                  <FiIcons.FiSend className="margin-r-5" color="grey" onClick={() => this.send(scene, true)} />
                                </span>
                              </Tooltip>
                              <Tooltip title="Send" className='float-right'>
                                <span className="icon">
                                  <FiIcons.FiSend className="margin-r-5" onClick={() => this.send(scene)} />
                                </span>
                              </Tooltip>
                              <Tooltip title="View/Edit Scene" className='float-right margin-r-5'>
                                <span className="icon">
                                  <Link to={`/scenes/${groupId}/${sceneId}`}><FiIcons.FiEdit /></Link>
                                </span>
                              </Tooltip>
                            </td>
                          </tr>
                        )
                      })
                    })}
                  </tbody>
                </Table>
              )
            }
          </Tab>
        </Tabs>
        <ConfirmDlg ConfirmAction={this.confirmRemove} ref={this.changeConfirmDlg} />
      </>
    )
  };
}

export default SceneList;