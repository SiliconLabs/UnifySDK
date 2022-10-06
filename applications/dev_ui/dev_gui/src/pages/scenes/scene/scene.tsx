import React, { Fragment } from 'react';
import { Button, Card, Col, Row } from 'react-bootstrap';
import { Link } from 'react-router-dom';
import { Autocomplete, MenuItem, TextField, Tooltip } from '@mui/material';
import { SceneProps, SceneState } from './scene-types';
import { SceneItem } from '../scene-list/scene-list-types';
import * as FiIcons from 'react-icons/fi';
import ConfirmDlg from '../../../components/confirm-dlg/confirm-dlg';
import SceneCommandDlg from '../scene-command-dlg/scene-command-dlg';
import { toast } from 'react-toastify';

class Scene extends React.Component<SceneProps, SceneState> {
  constructor(props: SceneProps) {
    super(props);
    this.state = {
      Scene: this.getScene(props, props.GroupID, props.SceneID),
      GetScene: this.getScene,
      Cluster: "",
      Cmd: "",
      IsSceneListReceived: false
    };
    this.changeConfirmDlg = React.createRef();
    this.editSceneDlg = React.createRef();
  }
  changeConfirmDlg: any;
  editSceneDlg: any;

  static getDerivedStateFromProps(props: SceneProps, state: SceneState) {
    let scene: any;
    if (props.SceneList != undefined && Object.keys(props.SceneList).length && !state.IsSceneListReceived) {
      state.IsSceneListReceived = true;
      if (props.GroupID !== "new" && props.SceneID !== "new")
        scene = state.GetScene(props, props.GroupID, props.SceneID);
    }
    return {
      Scene: scene ?? (state.Scene?.GroupID === undefined && state.Scene?.SceneID === undefined ? state.GetScene(props) : state.Scene)
    };
  }

  getScene(props: SceneProps, groupID: string, sceneID: string) {
    if (groupID && sceneID && props.SceneList && props.SceneList[groupID] && props.SceneList[groupID][sceneID])
      return props.SceneList[groupID][sceneID]
    else {
      let groupId = props.GroupList?.length > 0 ? props.GroupList[0].GroupId : undefined;
      if (groupId === undefined) return {} as SceneItem;
      let groupScenes = (props.SceneList && props.SceneList[groupId]) ? Object.keys(props.SceneList[groupId]) : undefined;
      return {
        GroupID: groupId,
        SceneID: groupScenes && groupScenes.length > 0 ? Number(groupScenes[groupScenes.length - 1]) + 1 : null,
        SceneName: "",
        SceneTableExtensions: {}
      } as SceneItem
    }
  }

  handleGroupChange = (event: any) => {
    let val = event.target.value?.match(/[0-9]{1,8}/g);
    val = val && val[0] !== undefined ? Number(val[0]) : null;
    this.setGroup(val);
  }

  onChangeAuto = (event: any, newValue: any) => {
    this.setGroup(newValue?.GroupId ?? newValue ?? null);
  }

  setGroup = (groupId: number) => {
    let scene = this.state.Scene;
    scene.GroupID = groupId;
    let groupScenes = this.props.SceneList && groupId !== undefined && this.props.SceneList[groupId] && Object.keys(this.props.SceneList[groupId]);
    scene.SceneID = groupScenes?.length > 0 ? Number(groupScenes[groupScenes.length - 1]) + 1 : scene.SceneID;
    this.setState({ Scene: scene });
  }

  handleNameChange = (event: any) => {
    let scene = this.state.Scene;
    scene.SceneName = event.target.value;
    this.setState({ Scene: scene });
  }

  handleIdChange = (event: any) => {
    let scene = this.state.Scene;
    let val = event.target.value?.match(/[0-9]{1,8}/g);
    val = val && val[0] !== undefined ? Number(val[0]) : null;
    scene.SceneID = val;
    this.setState({ Scene: scene });
  }

  save = () => {
    if (this.props.GroupID !== "new")
      this.props.SocketServer.send(JSON.stringify(
        {
          type: "save-scene",
          data: {
            GroupID: this.state.Scene.GroupID,
            SceneID: this.state.Scene.SceneID,
            Payload: this.state.Scene
          }
        }));
  }

  edit = (clusterType?: string, command?: string, payload?: any) => {
    if (this.state.Scene.GroupID === undefined || this.state.Scene.SceneID === undefined || this.state.Scene.GroupID === null || this.state.Scene.SceneID === null) {
      toast("'GroupId' and 'SceneId' are required", { type: "warning" });
      return;
    }
    this.editSceneDlg.current.updateState(clusterType, command, payload);
  }

  remove(cluster: string, cmd: string) {
    this.setState({ Cluster: cluster, Cmd: cmd }, () => {
      this.changeConfirmDlg?.current.update(
        `Remove ${this.state.Cluster} -> [${this.state.Cmd}]`,
        `Are you sure, you want to remove this command?`
      );
    });
  }

  sendScene = () => {
    if (!this.state.Scene?.SceneTableExtensions)
      return;
    Object.keys(this.state.Scene.SceneTableExtensions).forEach(cluster => {
      Object.keys(this.state.Scene.SceneTableExtensions[cluster])?.forEach(cmd => {
        this.sendCommand(cluster, cmd, this.state.Scene.SceneTableExtensions[cluster][cmd]);
      });
    });
  }

  sendCommand = (clusterType: string, cmd: string, payload: any) => {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "run-group-command",
        data: {
          GroupId: this.state.Scene.GroupID,
          ClusterType: clusterType,
          Cmd: cmd,
          Payload: payload
        }
      }))
  }

  confirmRemove = () => {
    let scene = this.state.Scene;
    delete (scene.SceneTableExtensions[this.state.Cluster][this.state.Cmd]);
    if (!Object.keys(scene.SceneTableExtensions[this.state.Cluster]).length)
      delete (scene.SceneTableExtensions[this.state.Cluster]);
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "save-scene",
        data: {
          GroupID: this.state.Scene.GroupID,
          SceneID: this.state.Scene.SceneID,
          Payload: this.state.Scene
        }
      }))
  }

  render() {
    return (
      <>
        <h3>{`Group: [ID: ${this.state.Scene.GroupID ?? ""}] ${this.props.GroupList.find(i => i.GroupId === this.state.Scene.GroupID)?.GroupName ?? ""}, Scene: ${this.state.Scene.SceneName ?? ""} [ID: ${this.state.Scene.SceneID ?? ""}]`}</h3>
        <Row>
          <div className='col-sm-12 margin-b-10'>
            <Link to={`/scenes`} className="margin-r-10">
              <Button variant="outline-primary">Back</Button>
            </Link>
            {!this.props.IsConnected ? <></>
              : <>
                {
                  this.props.GroupList?.length && this.props.GroupID === "new"
                    ? <Autocomplete
                      selectOnFocus
                      defaultValue={this.props.GroupList?.find(i => i.GroupId === this.state.Scene.GroupID) || {}}
                      getOptionLabel={(option) => `${option?.GroupId}`}
                      options={this.props.GroupList}
                      onChange={this.onChangeAuto}
                      onKeyDownCapture={(event: any) => {
                        if (event.keyCode !== 9 && event.keyCode !== 8 && event.keyCode !== 46 && (isNaN(event.key) || event.keyCode === 13)) {
                          event.stopPropagation();
                          event.preventDefault();
                        }
                      }}

                      freeSolo
                      className='display-inline'
                      renderInput={(params) => <TextField {...params}
                        onChange={this.handleGroupChange}
                        size="small" variant="outlined" label="Group ID" className="col-sm-2 margin-l-5 margin-r-10" onFocus={(event) => event.target.select()} />}
                    />
                    : <TextField size="small" disabled={this.props.GroupID !== "new"} variant="outlined" label="Group ID" className="col-sm-2 margin-l-5 margin-r-10" value={this.state.Scene.GroupID ?? null} onChange={this.handleGroupChange} onFocus={(event) => event.target.select()} />

                }
                <TextField size="small" className="col-sm-2 margin-l-5 margin-r-10" label="Scene ID" disabled={this.props.GroupID !== "new"} value={this.state.Scene.SceneID ?? null} onChange={this.handleIdChange} variant="outlined" onFocus={(event) => event.target.select()}
                  onKeyDownCapture={(event: any) => {
                    if (event.keyCode !== 9 && event.keyCode !== 8 && event.keyCode !== 46 && (isNaN(event.key) || event.keyCode === 13)) {
                      event.stopPropagation();
                      event.preventDefault();
                    }
                  }} />
                <TextField size="small" className="col-sm-3 margin-l-5" label="Scene Name" onBlur={this.save} value={this.state.Scene.SceneName} onChange={this.handleNameChange} variant="outlined" onFocus={(event) => event.target.select()} />
                <Button variant="primary" className="margin-h-10" onClick={() => this.edit()}>Add Command</Button>
                <Button hidden={!this.state.Scene?.SceneTableExtensions || !Object.keys(this.state.Scene.SceneTableExtensions)?.length} variant="outline-primary" className="margin-h-10" onClick={this.sendScene}>
                  Send
                </Button>
              </>
            }
          </div>
        </Row>
        {
          !this.props.IsConnected
            ? <Row>
              <Col xs={12} className="text-center">
                <span className="no-content">No Conection</span>
              </Col>
            </Row>
            : this.state.Scene && this.state.Scene.SceneTableExtensions && Object.keys(this.state.Scene.SceneTableExtensions)?.map((cluster, clusterIndex) => {
              return Object.keys(this.state.Scene.SceneTableExtensions[cluster])?.map((cmd: any, cmdIndex: number) => {
                return <Fragment key={`${clusterIndex}-${cmdIndex}`}>
                  <Card>
                    <Card.Header>{cluster} -&gt; [{cmd}]
                      <Tooltip title="Remove" className='float-right'>
                        <span className="icon">
                          <FiIcons.FiTrash2 className="margin-h-5" color="#dc3545" onClick={() => this.remove(cluster, cmd)} />
                        </span>
                      </Tooltip>
                      <Tooltip title="Send" className='float-right'>
                        <span className="icon">
                          <FiIcons.FiSend className="margin-h-5" onClick={() => this.sendCommand(cluster, cmd, this.state.Scene.SceneTableExtensions[cluster][cmd])} />
                        </span>
                      </Tooltip>
                      <Tooltip title="View/Edit" className='float-right'>
                        <span className="icon">
                          <FiIcons.FiEdit className="margin-h-5" onClick={() => this.edit(cluster, cmd, this.state.Scene.SceneTableExtensions[cluster][cmd])} />
                        </span>
                      </Tooltip>
                    </Card.Header>
                    <Card.Body className='inline no-padding' hidden={!Object.keys(this.state.Scene.SceneTableExtensions[cluster][cmd]).length}>
                      <div className='col-sm-12 padding-v-20 line-h-20'>
                        {JSON.stringify(this.state.Scene.SceneTableExtensions[cluster][cmd])}
                      </div>
                    </Card.Body>
                  </Card><br />
                </Fragment>
              })
            })
        }
        <ConfirmDlg ConfirmAction={this.confirmRemove} ref={this.changeConfirmDlg} />
        <SceneCommandDlg ref={this.editSceneDlg} SocketServer={this.props.SocketServer} Scene={this.state.Scene} />
      </>
    )
  }
}

export default Scene