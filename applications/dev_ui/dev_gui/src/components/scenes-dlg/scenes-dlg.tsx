import * as React from 'react';
import { Button, Card, Form, Modal } from 'react-bootstrap';
import { Scene, ScenesDlgProps, ScenesDlgState } from './scenes-dlg-types';
import { MenuItem, TextField } from '@material-ui/core';
import ConfirmDlg from '../confirm-dlg/confirm-dlg';


class ScenesDlg extends React.Component<ScenesDlgProps, ScenesDlgState> {
    constructor(props: ScenesDlgProps) {
        super(props);
        this.state = {
            Unid: "",
            ShowModal: false,
            Scenes: [],
            ProcessingScene: null,
        };
        this.processingScene = null;
        this.changeConfirmDlg = React.createRef();
    }
    changeConfirmDlg: any;

    processingScene: Scene | null;

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    updateState(unid: string, scenes: any[]) {
        this.setState({ Unid: unid, Scenes: scenes, ShowModal: true });
        if (scenes && scenes.length) {
            this.setState({ ProcessingScene: JSON.parse(JSON.stringify(scenes[0])) });
        }
    }

    parseBitmapValue(bitmapArray: any[], value: any) {
        if (!bitmapArray || !bitmapArray.length || value === null || value === undefined || isNaN(value))
            return;
        let strValue = value.toString(2);
        bitmapArray.forEach((item) => {
            let maskDecValue = 0;
            let mask = item.mask.toString(2);
            if (item.offset < strValue.length) {
                let maskLength = mask.substr(0, mask.length - item.offset).length;
                if (strValue.length < (maskLength + item.offset))
                    strValue = "0".repeat(maskLength + item.offset - strValue.length) + strValue;
                let maskBinValue = strValue.substr(strValue.length - (maskLength + item.offset), maskLength);
                maskDecValue = parseInt(maskBinValue, 2);
            }
            item.value = item.type === "boolean" ? maskDecValue === 1 : maskDecValue;
        });
    }

    handleSceneChange = (event: any) => {
        this.setState({ ProcessingScene: JSON.parse(JSON.stringify(this.state.Scenes.find(i => i.SceneID === event.target.value))) });
    }

    handleChange = (event: any) => {
        this.updateAttr(event.target.name, event.target.value);
    };

    handleCheckboxChange = (event: any) => {
        this.updateAttr(event.target.name, event.target.checked);
    };

    handleBitmapChange(attr: any, event: any) {
        let strValue = "";
        attr.bitmap.forEach((item: any) => {
            let mask = item.mask.toString(2);
            let maskLength = mask.substr(0, mask.length - item.offset).length;
            if (item.name === event.target.name)
                item.value = item.type === "boolean" ? event.target.checked : event.target.value;
            let itemStrValue = item.type === "boolean" ? (item.value ? "1" : "0") : item.value.toString(2);
            strValue = "0".repeat(maskLength - itemStrValue.length) + itemStrValue + strValue;
        });
        this.updateAttr(attr.name, parseInt(strValue, 2) || 0);
    }

    addScene = () => {
        let maxIndex = -1;
        this.state.Scenes && this.state.Scenes.length && this.state.Scenes.forEach((i) => maxIndex = maxIndex > i.SceneID ? maxIndex : i.SceneID);
        let scene = {
            GroupId: 0,
            SceneID: maxIndex + 1,
            TransitionTime: 10,
            TransitionTime100ms: 0,
            SceneName: "",
            SceneTableExtensions: {
                [this.props.ClusterType]: {}
            },
            IsNew: true
        }
        this.setState({ ProcessingScene: scene });
    }


    addCancel = () => {
        this.setState({ ProcessingScene: JSON.parse(JSON.stringify(this.state.Scenes[0])) });
    }


    delete = () => {
        if (!this.state.ProcessingScene)
            return;

        if ((this.state.ProcessingScene as any).IsNew)
            this.setState({ ProcessingScene: JSON.parse(JSON.stringify(this.state.Scenes[0])) });
        else
            this.changeConfirmDlg?.current.update(
                `Delete Scene ${this.state.ProcessingScene.SceneName}[${this.state.ProcessingScene.SceneID}]`,
                `Are you sure, you want to delete this scene?`
            );
    }

    confirmRemove = () => {
        if (this.state.ProcessingScene)
            this.props.SocketServer.send(JSON.stringify({
                type: "run-cluster-scene-command",
                data: {
                    Unid: this.state.Unid,
                    Cmd: "RemoveScene",
                    Payload: {
                        SceneID: this.state.ProcessingScene.SceneID
                    }
                }
            }));
    }

    updateAttr(name: string, value: any) {
        let scene = { ...this.state.ProcessingScene } as Scene;
        if (value === null || value === undefined || value === "" || value === false)
            delete (scene.SceneTableExtensions[this.props.ClusterType][name]);
        else
            scene.SceneTableExtensions[this.props.ClusterType][name] = value;
        this.setState({ ProcessingScene: scene });
    }

    saveScene() {
        if (!this.state.ProcessingScene?.SceneName || !Object.keys(this.state.ProcessingScene.SceneTableExtensions[this.props.ClusterType]).length)
            return;
        this.props.SocketServer.send(JSON.stringify({
            type: "run-cluster-scene-command",
            data: {
                Unid: this.state.Unid,
                Cmd: (this.state.ProcessingScene as any)["IsNew"] ? "AddScene" : "StoreScene",
                Payload: {
                    GroupID: 0,
                    SceneID: this.state.ProcessingScene.SceneID,
                    TransitionTime: this.state.ProcessingScene.TransitionTime,
                    SceneName: this.state.ProcessingScene.SceneName,
                    ExtensionFieldSets: {
                        [this.props.ClusterType]: { ...this.state.ProcessingScene.SceneTableExtensions[this.props.ClusterType] }
                    }
                }
            }
        }));
        this.setState({ ShowModal: false });
    }

    handleNameChange = (event: any) => {
        let scene = { ...this.state.ProcessingScene } as Scene;
        scene.SceneName = event.target.value;
        this.setState({ ProcessingScene: scene });
    }

    render() {
        let sceneAttrs = this.state.ProcessingScene?.SceneTableExtensions[this.props.ClusterType] || {};
        return (
            <Modal show={this.state.ShowModal} size="lg" onHide={() => this.toggleModal(false)}>
                <Modal.Header closeButton>
                    <Modal.Title>Scenes</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    <div className="col-sm-12">
                        <div className="flex">
                            <div hidden={!this.state.Scenes || !this.state.Scenes.length || (this.state.ProcessingScene as any)["IsNew"]} className="col-sm-6">
                                <TextField size="small" className="flex-input" select label="Scenes" name="Scenes" value={this.state.ProcessingScene?.SceneID} onChange={this.handleSceneChange} variant="outlined">
                                    {this.state.Scenes.map((option: any) => {
                                        return <MenuItem key={option.SceneID} value={option.SceneID}>
                                            {option.SceneName}
                                        </MenuItem>
                                    })}
                                </TextField>
                            </div>
                            <div hidden={!this.state.ProcessingScene || !(this.state.ProcessingScene as any)["IsNew"]} className="col-sm-6">
                                <TextField size="small" className="flex-input" label="Scenes" name="Scenes" onChange={this.handleNameChange} variant="outlined" />
                            </div>
                            <div>
                                <Button className="margin-h-5" variant="outline-primary" onClick={this.delete}>Delete</Button>
                                <Button className="margin-h-5" hidden={!this.state.ProcessingScene || (this.state.ProcessingScene as any)["IsNew"]} variant="outline-primary" onClick={this.addScene}>Create</Button>
                                {/* <Button hidden={!this.state.ProcessingScene || !(this.state.ProcessingScene as any)["IsNew"]} variant="outline-primary" onClick={this.addCancel}>Remove</Button> */}
                            </div>
                        </div>
                        <hr />
                        <div hidden={!this.state.ProcessingScene}>
                            {this.props.SceneAttrs.map((i: any, index: number) => {
                                switch (i.type) {
                                    case "boolean":
                                        return <div key={index} className="col-sm-6 inline margin-v-10">
                                            <div className="col-sm-12">
                                                <Form.Label column sm="11">
                                                    <div className="check-container">
                                                        <Form.Check name={i.name} checked={sceneAttrs[i.name] ?? false} onChange={this.handleCheckboxChange} />
                                                    </div>
                                                    {i.name}
                                                </Form.Label>
                                            </div>
                                        </div>
                                    case "bitmap":
                                        if (i.bitmap && i.bitmap.length)
                                            this.parseBitmapValue(i.bitmap, sceneAttrs[i.name])
                                        return <Card key={index} className="inline margin-v-10">
                                            <Card.Header>{i.name}: {sceneAttrs[i.name]}</Card.Header>
                                            <Card.Body>
                                                {i.bitmap && i.bitmap.map((b: any, bIndex: number) => {
                                                    switch (b.type) {
                                                        case "enum":
                                                            return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                                                {b.enum
                                                                    ? <TextField size="small" className="writable flex-input" fullWidth={true} select label={b.name} name={b.name} value={b.value ?? 0} onChange={this.handleBitmapChange.bind(this, i)} variant="outlined">
                                                                        {b.enum.map((option: any) => {
                                                                            return <MenuItem key={option.value} value={option.value}>
                                                                                {option.name}
                                                                            </MenuItem>
                                                                        })}
                                                                    </TextField>
                                                                    : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label={b.name} value={b.value ?? 0} variant="outlined" />
                                                                }
                                                            </div>
                                                        case "boolean":
                                                            return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                                                <div className={`col-sm-12 ${i.writable ? "writable" : ""}`}>
                                                                    <Form.Label column sm="11">
                                                                        <div className="check-container">
                                                                            <Form.Check name={b.name} checked={b.value ?? false} onChange={this.handleBitmapChange.bind(this, i)} />
                                                                        </div>
                                                                        {b.name}</Form.Label>
                                                                </div>
                                                            </div>
                                                        default: return <></>
                                                    }
                                                })
                                                }
                                            </Card.Body>
                                        </Card>
                                    case "enum":
                                        return <div key={index} className="col-sm-6 inline margin-v-10">
                                            <TextField size="small" className="writable flex-input" fullWidth={true} select label={i.name} name={i.name} value={sceneAttrs[i.name] ?? ""} onChange={this.handleChange} variant="outlined">
                                                {i.enum.map((option: any) => {
                                                    return <MenuItem key={option.name} value={option.name}>
                                                        {option.name}
                                                    </MenuItem>
                                                })}
                                            </TextField>
                                        </div>
                                    default:
                                        return <div key={index} className="col-sm-6 inline margin-v-10">
                                            <TextField size="small" className="writable flex-input" fullWidth={true} label={i.name} name={i.name} variant="outlined" type={i.type} value={sceneAttrs[i.name]} onChange={this.handleChange} />
                                        </div>
                                }
                            })}
                        </div>
                    </div>
                </Modal.Body>
                <Modal.Footer>
                    <Button variant="outline-primary" onClick={() => this.saveScene()}>
                        Save
                    </Button>
                    <Button variant="primary" onClick={() => this.toggleModal(false)}>
                        Cancel
                    </Button>
                </Modal.Footer>
                <ConfirmDlg ConfirmAction={this.confirmRemove} ref={this.changeConfirmDlg}></ConfirmDlg>
            </Modal >
        );
    }
}

export default ScenesDlg