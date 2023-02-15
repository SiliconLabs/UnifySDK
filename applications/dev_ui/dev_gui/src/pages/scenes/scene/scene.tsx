import React, { Fragment } from 'react';
import { Button, Card, Col, Row } from 'react-bootstrap';
import { Link } from 'react-router-dom';
import { Autocomplete, MenuItem, TextField, Tooltip } from '@mui/material';
import { SceneItem, SceneProps, SceneState } from './scene-types';
import ConfirmDlg from '../../../components/confirm-dlg/confirm-dlg';
import { ClusterTypeAttrs } from '../../../cluster-types/cluster-type-attributes';
import SceneAttributes from '../scene-attributes/scene-attributes';
import AddAttribute from '../../../components/add-attribute/add-attribute';
import { ClusterTypes } from '../../../cluster-types/cluster-types';
import './scene.css';

class Scene extends React.Component<SceneProps, SceneState> {
  constructor(props: SceneProps) {
    super(props);
    this.state = {
      Scene: this.getScene(props),
      GetScene: this.getScene,
      Cluster: "",
      ClusterToAdd: "",
      IsEdit: props.GroupID === "new" || props.SceneID === "new",
      AvailableClusters: []
    };

    this.changeConfirmDlg = React.createRef();
  }
  changeConfirmDlg: any;

  static getDerivedStateFromProps(props: SceneProps, state: SceneState) {
    let scene = state.Scene ?? state.GetScene(props);
    let availableClusters = Object.keys(ClusterTypeAttrs).filter(cluster => !(scene?.SceneTableExtensions && scene?.SceneTableExtensions[cluster]) && ClusterTypeAttrs[cluster].server.attributes.find((attr: any) => attr.sceneRequired));
    return {
      Scene: scene,
      AvailableClusters: availableClusters,
      ClusterToAdd: (state.ClusterToAdd && availableClusters.indexOf(state.ClusterToAdd) > -1) ? state.ClusterToAdd : ((availableClusters && availableClusters[0]) || "")
    };
  }

  getScene(props: any) {
    let scene: any = null;
    if (props.Unid && props.Ep) {
      if (props.GroupID && props.SceneID && props.GroupID !== "new" && props.SceneID !== "new") {
        let node = props.NodeList.find((i: any) => i.Unid === props.Unid);
        let scenes = node && node.ep && node.ep[props.Ep]?.Clusters?.Scenes?.Attributes?.SceneTable?.Reported;
        if (scenes && scenes.length) {
          scene = scenes.find((i: any) => i.GroupID === Number(props.GroupID) && i.SceneID === Number(props.SceneID));
          scene = scene && JSON.parse(JSON.stringify(scene));
        }
      } else if (props.GroupID === "new" && props.SceneID === "new") {
        scene = {
          GroupID: 0,
          SceneID: 0,
          SceneName: "",
          TransitionTime: 0,
          TransitionTime100ms: 0,
          SceneTableExtensions: {},
          IsEdit: true
        } as SceneItem;
      }
    }
    return scene;
  }

  handleGroupChange = (event: any) => {
    let val = event.target.value?.match(/[0-9]{1,8}/g);
    val = val && val[0] !== undefined ? Number(val[0]) : 0;
    this.setGroup(val);
  }

  onChangeAuto = (event: any, newValue: any) => {
    this.setGroup(newValue?.GroupId ?? newValue ?? 0);
  }

  setGroup = (groupId: number) => {
    let scene = this.state.Scene;
    scene.GroupID = groupId;
    this.setState({ Scene: scene });
  }

  handleSceneChange = (event: any) => {
    let scene = this.state.Scene as any;
    if (event.target.type === "number") {
      let val = event.target.value?.match(/[0-9]{1,8}/g);
      val = val && val[0] !== undefined ? Number(val[0]) : 0;
      scene[event.target.name] = val;
    } else
      scene[event.target.name] = event.target.value;
    this.setState({ Scene: scene });
  }

  handleIdChange = (event: any) => {
    let scene = this.state.Scene;
    let val = event.target.value?.match(/[0-9]{1,8}/g);
    val = val && val[0] !== undefined ? Number(val[0]) : null;
    scene.SceneID = val;
    this.setState({ Scene: scene });
  }

  confirmSave() {
    if (this.props.GroupID === "new" && this.props.SceneID === "new") {
      let node = this.props.NodeList.find((i: any) => i.Unid === this.props.Unid);
      let scenes = node && node.ep && node.ep[this.props.Ep]?.Clusters?.Scenes?.Attributes?.SceneTable?.Reported;
      if (scenes.filter((i: any) => i.GroupID === this.state.Scene.GroupID && i.SceneID === this.state.Scene.SceneID)?.length) {
        this.changeConfirmDlg?.current.update(
          `Save GroupID:[${this.state.Scene.GroupID}] SceneID:[${this.state.Scene.SceneID}]`,
          `Key {GroupID, SceneID} already exists. Do you want to overwrite it?`,
          this.save);
        return;
      }
    }
    this.save();
  }

  save = () => {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "run-cluster-command",
        data: {
          Unid: `${this.props.Unid}/${this.props.Ep}`,
          ClusterType: ClusterTypes.Scenes,
          Cmd: "AddScene",
          Payload: {
            GroupID: this.state.Scene.GroupID,
            SceneID: this.state.Scene.SceneID,
            TransitionTime: this.state.Scene.TransitionTime,
            TransitionTime100ms: this.state.Scene.TransitionTime100ms,
            SceneName: this.state.Scene.SceneName,
            ExtensionFieldSets: this.state.Scene.SceneTableExtensions
          }
        }
      }));
    history.back();
  }

  addCluster = () => {
    let scene = this.state.Scene;
    if (!scene.SceneTableExtensions[this.state.ClusterToAdd])
      scene.SceneTableExtensions[this.state.ClusterToAdd] = {};
    this.setState({ Scene: scene, AvailableClusters: Object.keys(ClusterTypeAttrs).filter(cluster => !(scene?.SceneTableExtensions && scene?.SceneTableExtensions[cluster]) && ClusterTypeAttrs[cluster].server.attributes.find((attr: any) => attr.sceneRequired)) });
  }

  confirmRemove(cluster: string) {
    this.setState({ Cluster: cluster }, () => {
      this.changeConfirmDlg?.current.update(
        `Remove ${this.state.Cluster}`,
        `Are you sure, you want to remove this cluster?`,
        this.remove
      );
    });
  }

  remove = () => {
    let scene = this.state.Scene;
    delete (scene.SceneTableExtensions[this.state.Cluster]);
  }

  recall = () => {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "run-cluster-command",
        data: {
          Unid: `${this.props.Unid}/${this.props.Ep}`,
          ClusterType: ClusterTypes.Scenes,
          Cmd: "RecallScene",
          Payload: {
            GroupID: this.state.Scene.GroupID,
            SceneID: this.state.Scene.SceneID,
            TransitionTime: this.state.Scene.TransitionTime
          }
        }
      }));
  }

  addAttribute = (cluster: string, attr: any) => {
    if (attr && attr.name) {
      this.state.Scene.SceneTableExtensions[cluster][attr.name] = attr.type === "struct" ? {} : attr.type === "number" ? 0 : "";
      this.setState({ Scene: this.state.Scene });
    }
  }

  removeAttribute = (cluster: string, attr: any) => {
    delete (this.state.Scene.SceneTableExtensions[cluster][attr]);
    this.setState({ Scene: this.state.Scene });
  }

  render() {
    return (
      <>
        <h3>{`Group: [ID: ${this.state.Scene?.GroupID ?? ""}] ${(this.state.Scene && this.props.GroupList.find(i => i.GroupId === this.state.Scene.GroupID)?.GroupName) ?? ""}, Scene: ${this.state.Scene?.SceneName ?? ""} [ID: ${this.state.Scene?.SceneID ?? ""}]`}</h3>
        <Row>
          <div className='col-sm-12 margin-b-10 inline'>
            <Link to={`/scenes/${this.props.Unid}/${this.props.Ep}`} className="margin-r-10">
              <Button variant="outline-primary">Back</Button>
            </Link>
            {!this.state.Scene || !this.props.IsConnected
              ? <Row>
                <Col xs={12} className="text-center">
                  <span className="no-content">No Content</span>
                </Col>
              </Row>
              : <>
                <Button hidden={this.props.GroupID === "new" || this.props.SceneID === "new" || !this.state.Scene?.SceneTableExtensions || !Object.keys(this.state.Scene.SceneTableExtensions)?.length} variant="outline-primary" className="margin-h-10" onClick={this.recall}>
                  Recall
                </Button>
                {this.state.IsEdit
                  ? <>
                    <Button hidden={!this.state.IsEdit} variant="outline-primary" className="float-right" onClick={() => this.setState({ Scene: this.getScene(this.props), IsEdit: false })}>
                      Cancel
                    </Button>
                    <Button hidden={!this.state.IsEdit} variant="primary" className="margin-r-10 float-right" onClick={() => this.confirmSave()}>
                      Save
                    </Button>
                  </>
                  : <Button hidden={this.state.IsEdit} variant="primary" className="margin-r-10 float-right" onClick={() => this.setState({ IsEdit: true })}>
                    Edit
                  </Button>
                }
              </>
            }
          </div>
        </Row>
        {!this.state.Scene || !this.props.IsConnected
          ? <></>
          : <><Row>
            <div className="col-sm-12 inline margin-v-10">
              <Autocomplete hidden={this.props.GroupID !== "new"}
                selectOnFocus
                value={this.props.GroupList?.find(i => i.GroupId === this.state.Scene.GroupID) || {}}
                getOptionLabel={(option) => `${option && option.GroupId !== undefined ? `${option.GroupName} [ID: ${option.GroupId}]` : 0}`}
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
                  variant="outlined" label="Group ID" className="col-sm-2 margin-r-10 small" onFocus={(event) => event.target.select()} />}
              />
              <TextField size="small" className="col-sm-2 margin-r-10" label="Scene ID" hidden={this.props.GroupID !== "new"} value={this.state.Scene.SceneID ?? null} onChange={this.handleIdChange} variant="outlined" onFocus={(event) => event.target.select()}
                onKeyDownCapture={(event: any) => {
                  if (event.keyCode !== 9 && event.keyCode !== 8 && event.keyCode !== 46 && (isNaN(event.key) || event.keyCode === 13)) {
                    event.stopPropagation();
                    event.preventDefault();
                  }
                }} />
              <TextField size="small" disabled={!this.state.IsEdit} className="col-sm-2 margin-r-10" label="Scene Name" value={this.state.Scene.SceneName} name="SceneName" onChange={this.handleSceneChange} variant="outlined" onFocus={(event) => event.target.select()} />
              <TextField size="small" disabled={!this.state.IsEdit} className="col-sm-2 margin-r-10" label="Transition Time" value={this.state.Scene.TransitionTime} name="TransitionTime" onChange={this.handleSceneChange} variant="outlined" onFocus={(event) => event.target.select()} type="number" />
              <TextField size="small" disabled={!this.state.IsEdit} className="col-sm-2 margin-r-10" label="Transition Time 100ms" value={this.state.Scene.TransitionTime100ms} name="TransitionTime100ms" onChange={this.handleSceneChange} variant="outlined" onFocus={(event) => event.target.select()} type="number" />
            </div>
          </Row>
            <Row hidden={!this.state.IsEdit || !this.state.AvailableClusters || !this.state.AvailableClusters.length}>
              <div className="col-sm-12 inline margin-b-10">
                <TextField size="small" className="col-sm-4 margin-r-10" fullWidth={true} select label="Cluster Type" name="ClusterType" value={this.state.ClusterToAdd} onChange={(event: any) => this.setState({ ClusterToAdd: event?.target?.value })} variant="outlined">
                  {this.state.AvailableClusters?.map((cluster: any, index: number) => {
                    return <MenuItem key={index} value={cluster}>
                      {cluster}
                    </MenuItem>
                  })}
                </TextField>
                <Button variant="primary" onClick={() => this.addCluster()} disabled={!this.state.ClusterToAdd}>Add Cluster</Button>
              </div>
            </Row>
          </>
        }
        {
          !this.props.IsConnected
            ? <Row>
              <Col xs={12} className="text-center">
                <span className="no-content">No Conection</span>
              </Col>
            </Row>
            : this.state.Scene && this.state.Scene.SceneTableExtensions && Object.keys(this.state.Scene.SceneTableExtensions)?.map((cluster, clusterIndex) => {
              let attrs = Object.keys(this.state.Scene.SceneTableExtensions[cluster]) || [];
              return <Fragment key={clusterIndex}>
                <Card>
                  <Card.Header className='inline'><span className={this.state.IsEdit ? "line27" : ""}><b className='mr-3'>{cluster}</b></span>
                    {this.state.IsEdit
                      ? <>
                        <Tooltip title="Remove Cluster">
                          <Button hidden={!this.state.IsEdit} size="sm" variant="outline-primary" className="float-right" onClick={() => this.confirmRemove(cluster)}>Remove</Button>
                        </Tooltip>
                        <AddAttribute Cluster={cluster} AddAttributeFunc={this.addAttribute} AttrList={this.state.Scene?.SceneTableExtensions && this.state.Scene.SceneTableExtensions[cluster]} />
                      </>
                      : <>
                      </>
                    }
                  </Card.Header>
                  {attrs.length
                    ? <Card.Body className='inline no-padding' hidden={!attrs.length}>
                      <div className="col-sm-12 cluster-attributes">
                        <SceneAttributes Scene={this.state.Scene} Cluster={cluster} IsEdit={this.state.IsEdit} RemoveAttribute={this.removeAttribute} />
                      </div>
                    </Card.Body>
                    : <></>
                  }
                </Card>
                <br />
              </Fragment>
            })
        }

        <ConfirmDlg ref={this.changeConfirmDlg} />
      </>
    )
  }
}

export default Scene