import React, { Fragment } from 'react';
import { Button, Card, Col, Row, Table } from 'react-bootstrap';
import { EpScenesProps, EpScenesState } from './ep-scenes-types';
import { Tooltip } from '@mui/material';
import * as FiIcons from 'react-icons/fi'
import ConfirmDlg from '../../../components/confirm-dlg/confirm-dlg';
import { Link } from 'react-router-dom';
import { ClusterTypes } from '../../../cluster-types/cluster-types';
import { SceneItem } from '../scene/scene-types';

export class EpScenes extends React.Component<EpScenesProps, EpScenesState> {
  constructor(props: EpScenesProps) {
    super(props);
    this.state = {
      ProcessingScene: {} as SceneItem
    }
    this.changeConfirmDlg = React.createRef();
  }
  changeConfirmDlg: any;
  
  remove(item: any) {
    this.setState({ ProcessingScene: item }, () => {
      this.changeConfirmDlg?.current.update(
        `Remove Scene [GroupID:${this.state.ProcessingScene.GroupID}][SceneID:${this.state.ProcessingScene.SceneID}]`,
        `Are you sure, you want to remove this scene?`,
        this.confirmRemove
      );
    });
  }

  confirmRemove = () => {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "run-cluster-command",
        data: {
          Unid: `${this.props.Unid}/${this.props.Ep}`,
          ClusterType: "Scenes",
          Cmd: "RemoveScene",
          Payload: {
            GroupID: this.state.ProcessingScene.GroupID,
            SceneID: this.state.ProcessingScene.SceneID
          }
        }
      }));
  }

  getAttributes = (scenes: any) => {
    return Object.keys(scenes).map((cluster: any, sindex: number) => <Fragment key={sindex}><b>{cluster}:</b> &#x7b;{Object.keys(scenes[cluster]).map((attr: any, aindex: number) => <Fragment key={aindex}><i>{attr}</i>: {JSON.stringify(scenes[cluster][attr])}{aindex === Object.keys(scenes[cluster]).length - 1 ? "" : ", "}</Fragment>)}&#x7d;<br /></Fragment>);
  }

  recall = (scene: SceneItem) => {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "run-cluster-command",
        data: {
          Unid: `${this.props.Unid}/${this.props.Ep}`,
          ClusterType: ClusterTypes.Scenes,
          Cmd: "RecallScene",
          Payload: {
            GroupID: scene.GroupID,
            SceneID: scene.SceneID,
            TransitionTime: scene.TransitionTime
          }
        }
      }));
  }

  render() {
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    let scenes: any[] = [];
    if (this.props.Unid && this.props.Ep) {
      let node = this.props.NodeList.find(i => i.Unid === this.props.Unid);
      scenes = node && node.ep && node.ep[this.props.Ep]?.Clusters?.Scenes?.Attributes?.SceneTable?.Reported
    }
    return (
      <>
        <h3>Scenes of {this.props.Unid}/{this.props.Ep}</h3>
        <Row>
          <div className='col-sm-12 margin-b-10 mr-3'>
            <Link to={`/scenes`} className="margin-r-10">
              <Button variant="outline-primary">Back</Button>
            </Link>
            <Link to={`/scenes/${this.props.Unid}/${this.props.Ep}/new/new`} className="margin-r-10">
              <Button variant="outline-primary">Add Scene</Button>
            </Link>
          </div>
        </Row>
        {(!scenes || !scenes.length)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          :
          (isMobile
            ? <div className='table-content'>
              {scenes.map((scene, index) =>
                <Card key={index} className="inline margin-v-10">
                  <Card.Header className='flex'>
                    <div className="col-sm-7">
                      <b><i>Group:</i></b> {`[ID: ${scene.GroupID}] ${this.props.GroupList.find(i => i.GroupId === scene.GroupID)?.GroupName ?? ""}`}
                    </div>
                    <div className='inline col-sm-5'>
                      <Tooltip title="Remove" className='float-right'>
                        <span className="icon">
                          <FiIcons.FiTrash2 color="#dc3545" onClick={() => this.remove(scene)} />
                        </span>
                      </Tooltip>
                      <Tooltip title="View/Edit Scene" className='float-right margin-r-5'>
                        <span className="icon">
                          <Link to={`/scenes/${this.props.Unid}/${this.props.Ep}/${scene.GroupID}/${scene.SceneID}`}><FiIcons.FiEdit /></Link>
                        </span>
                      </Tooltip>
                      <Tooltip title="Recall Scene" className='float-right margin-r-5' hidden={!scene?.SceneTableExtensions || !Object.keys(scene.SceneTableExtensions)?.length}>
                        <span className="icon">
                          <FiIcons.FiSend onClick={() => this.recall(scene)} />
                        </span>
                      </Tooltip>
                    </div>
                  </Card.Header>
                  <Card.Body>
                    <div className='col-sm-12'>
                      <div className="col-sm-12 inline"><b><i>Scene: </i></b>{`[ID: ${scene.SceneID}] ${scene.SceneName ?? ""}`}</div>
                      <div className="col-sm-6"><b><i>TransitionTime: </i></b>{scene.TransitionTime}</div>
                      <div className="col-sm-6"><b><i>TransitionTime100ms: </i></b>{scene.TransitionTime100ms}</div>
                      <div className="col-sm-12 inline"><b><i>Attributes: </i></b>{this.getAttributes(scene.SceneTableExtensions)}</div>
                    </div>
                  </Card.Body>
                </Card>
              )}
            </div>
            : <Table striped hover>
              <thead>
                <tr className="">
                  <th>Group</th>
                  <th>Scene</th>
                  <th>TransitionTime</th>
                  <th>TransitionTime100ms</th>
                  <th>Attributes</th>
                  <th className="wd-col-2">&ensp;</th>
                </tr>
              </thead>
              <tbody>
                {scenes.map((scene, index) =>
                  <tr key={index}>
                    <td>{`[ID: ${scene.GroupID}] ${this.props.GroupList.find(i => i.GroupId === scene.GroupID)?.GroupName ?? ""}`}</td>
                    <td>{`[ID: ${scene.SceneID}] ${scene.SceneName ?? ""}`}</td>
                    <td>{scene.TransitionTime}</td>
                    <td>{scene.TransitionTime100ms}</td>
                    <td>{this.getAttributes(scene.SceneTableExtensions)}</td>
                    <td>
                      <Tooltip title="Remove" className='float-right'>
                        <span className="icon">
                          <FiIcons.FiTrash2 color="#dc3545" onClick={() => this.remove(scene)} />
                        </span>
                      </Tooltip>
                      <Tooltip title="View/Edit Scene" className='float-right margin-r-5'>
                        <span className="icon">
                          <Link to={`/scenes/${this.props.Unid}/${this.props.Ep}/${scene.GroupID}/${scene.SceneID}`}><FiIcons.FiEdit /></Link>
                        </span>
                      </Tooltip>
                      <Tooltip title="Recall Scene" className='float-right margin-r-5' hidden={!scene?.SceneTableExtensions || !Object.keys(scene.SceneTableExtensions)?.length}>
                        <span className="icon">
                          <FiIcons.FiSend onClick={() => this.recall(scene)} />
                        </span>
                      </Tooltip>
                    </td>
                  </tr>
                )}
              </tbody>
            </Table>
          )
        }

        <ConfirmDlg ref={this.changeConfirmDlg} />
      </>
    )
  };
}

export default EpScenes;