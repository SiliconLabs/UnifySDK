import * as React from 'react';
import { Button, Card, Form, Modal } from 'react-bootstrap';
import { ClusterAttrProps, ClusterAttrState } from './cluster-attributes-types';
import "./cluster-attributes.css";
import * as AiIcons from 'react-icons/ai';
import * as MdIcons from 'react-icons/md';
import * as FiIcons from 'react-icons/fi';
import { InputAdornment, MenuItem, TextField, Tooltip } from '@material-ui/core';

class ClusterAttr extends React.Component<ClusterAttrProps, ClusterAttrState> {
    constructor(props: ClusterAttrProps) {
        super(props);
        this.state = {
            Unid: "",
            SupportedAttributes: {},
            PristineSupportedAttributes: {},
            ShowModal: false,
            HideUnSup: true,
            SupportedCommands: []
        };
        this.changedAttrList = new Map<string, any>();
    }
    changedAttrList: Map<string, any>;

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    updateState(unid: string, supAttr: any[], supportedCommands: string[]) {
        this.changedAttrList = new Map<string, any>();
        this.setState({
            Unid: unid,
            PristineSupportedAttributes: supAttr,
            SupportedAttributes: (JSON.parse(JSON.stringify(supAttr))),
            SupportedCommands: supportedCommands,
            ShowModal: true
        },
            () => this.validateModel());
    }

    handleChange = (event: any) => {
        this.updateAttr(event.target.name, event.target.value);
    };

    handleCheckboxChange = (event: any) => {
        this.updateAttr(event.target.name, event.target.checked);
    };

    handleBitmapChange(attr: any, isCheckBox: boolean, event: any) {
        let sup = this.state.SupportedAttributes[attr.name];
        sup.Reported[event.target.name] = isCheckBox ? event.target.checked : event.target.value;
        this.updateAttr(attr.name, sup.Reported);
    }

    updateAttr(name: string, value: any) {
        let supAttr = this.state.SupportedAttributes;
        supAttr[name].Reported = value;
        this.setState({ SupportedAttributes: supAttr });
        if (this.state.PristineSupportedAttributes[name]?.Reported === value)
            this.changedAttrList.delete(name);
        else
            this.changedAttrList.set(name, value);
    }

    validateModel() {
        let isValid = true;
        let supAttr = this.state.SupportedAttributes;
        Object.keys(this.state.SupportedAttributes).forEach((item: any) => {
            supAttr[item].isValid = this.validate(item);
            isValid = isValid && supAttr[item].isValid;
        });
        this.setState({ SupportedAttributes: supAttr });
        return isValid;
    }

    validateAttr(item: any) {
        let supAttr = this.state.SupportedAttributes;
        supAttr[item.name].isValid = this.validate(item.name);
        this.setState({ SupportedAttributes: supAttr });
    }

    validate(attrName: any) {
        let attr = this.props.ClusterAttr.find(i => i.name === attrName);
        return !(attr && attr.writable
            && ((attr.required && (this.state.SupportedAttributes[attrName].Reported === undefined || this.state.SupportedAttributes[attrName].Reported === null))
                || (attr.min !== undefined && attr.min !== null && Number(this.state.SupportedAttributes[attrName].Reported) < attr.min)
                || (attr.max !== undefined && attr.max !== null && Number(this.state.SupportedAttributes[attrName].Reported) > attr.max)));
    }

    saveAttr() {
        if (!this.validateModel()) return;
        this.changedAttrList.size && this.changedAttrList.forEach((value, key) => {
            this.props.SocketServer.send(JSON.stringify(
                {
                    type: "update-cluster-attr",
                    data: {
                        Unid: this.state.Unid,
                        ClusterType: this.props.ClusterType,
                        Payload: { [key]: value }
                    }
                }));
        });
        this.setState({ ShowModal: false });
    }

    getHintText = (item: any) => {
        let hints: string[] = [];
        if (item.min !== undefined || item.max !== undefined) {
            if (item.min !== undefined && item.max !== undefined)
                hints.push(`range: ${item.min}..${item.max}`);
            else if (item.min !== undefined)
                hints.push(`min: ${item.min}`)
            else hints.push(`max: ${item.max}`);
        }
        if (item.default !== undefined && item.default !== 'NaN' && !isNaN(item.default))
            hints.push(`default: ${item.type === "boolean" ? !!item.default : item.default}`);
        return <Form.Text className="hint">{hints.join("; ")}</Form.Text>
    }

    forceRead(attrName?: any) {
        this.props.SocketServer.send(JSON.stringify(
            {
                type: "force-read-cluster-attr",
                data: {
                    Unid: this.state.Unid,
                    ClusterType: this.props.ClusterType,
                    Payload: {
                        value:
                            attrName ? [attrName] : []
                    }
                }
            }));
    }

    getAdornment(name: string, type: string, isSupported: boolean) {
        let tooltip = <Tooltip title={`Force Read Attribute ${isSupported ? "" : "(maybe not supported)"}`}>
            <span className={`${isSupported ? "" : "disabled pointer"} icon`}>
                <MdIcons.MdRefresh onClick={() => this.forceRead(name)} />
            </span>
        </Tooltip>;
        return type === "boolean" || type === "bitmap"
            ? <div className="adornment">
                {tooltip}
            </div>
            : {
                endAdornment: <InputAdornment position="end">
                    {tooltip}
                </InputAdornment>
            }
    }

    render() {
        let keys = (this.state.SupportedAttributes && Object.keys(this.state.SupportedAttributes)) || [];
        let isForceReadAttrSupported = this.state.SupportedCommands && this.state.SupportedCommands.indexOf("ForceReadAttributes") > -1;
        let isWriteAttrSupported = this.state.SupportedCommands && this.state.SupportedCommands.indexOf("WriteAttributes") > -1;
        return (
            <Modal show={this.state.ShowModal} size="lg" onHide={() => this.toggleModal(false)}>
                <Modal.Header closeButton>
                    <Modal.Title>{this.props.ClusterType} Attribute(s) &nbsp;
                        {this.state.HideUnSup
                            ? <Tooltip title="Show All Attributes">
                                <span className="icon"><AiIcons.AiOutlineEye color="green" onClick={() => this.setState({ HideUnSup: false })} /></span>
                            </Tooltip>
                            : <Tooltip title="Hide Unsupported Attributes">
                                <span className="icon"><AiIcons.AiOutlineEyeInvisible color="green" onClick={() => this.setState({ HideUnSup: true })} /></span>
                            </Tooltip>}
                        <Tooltip title={`Force Read Attributes ${isForceReadAttrSupported ? "" : "(maybe not supported)"}`} className="margin-h-5">
                            <span className={`${isForceReadAttrSupported ? "" : "disabled pointer"} icon`}><MdIcons.MdRefresh onClick={() => this.forceRead()} /></span>
                        </Tooltip>
                        <div className="col-sm-12 writable-hint">
                            <div className="col-md-12">
                                <span className="writable"></span> - Writable Attributes
                            </div>
                            <div className="col-md-12" hidden={isWriteAttrSupported}>
                                <span>
                                    <FiIcons.FiAlertTriangle color="orange" fontSize="15px" />
                                </span> - "WriteAttributes" command is not advertised by the cluster, so this feature may not be supported
                            </div>
                            <div className="col-md-12" hidden={isForceReadAttrSupported}>
                                <span>
                                    <FiIcons.FiAlertTriangle color="orange" fontSize="15px" />
                                </span> - "ForceReadAttributes" command is not advertised by the cluster, so this feature may not be supported
                            </div>
                        </div>
                    </Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    <div className="col-sm-12 cluster-attributes">
                        {this.props.ClusterAttr && this.props.ClusterAttr.map((i: any, index: number) => {
                            let supported = keys.indexOf(i.name) > -1;
                            let adornment = this.getAdornment(i.name, i.type, isForceReadAttrSupported);
                            if (this.state.HideUnSup && !supported)
                                return <span key={index} hidden={true}></span>;
                            switch (i.type) {
                                case "boolean":
                                    return <div key={index} className="col-sm-6 inline margin-v-10">
                                        <div className={`col-sm-12 no-padding-r ${i.writable ? "writable" : ""}`}>
                                            <Form.Label column sm="11" className={supported ? "" : "disabled"}>
                                                <div className="check-container">
                                                    {i.writable && supported
                                                        ? <Form.Check name={i.name} disabled={!supported} defaultChecked={this.state.SupportedAttributes[i.name]?.Reported} onChange={this.handleCheckboxChange} />
                                                        : <Form.Check name={i.name} readOnly={true} disabled={!supported} checked={this.state.SupportedAttributes[i.name]?.Reported} />}
                                                </div>
                                                {i.name}</Form.Label>
                                            {adornment}
                                        </div>
                                        {this.getHintText(i)}
                                    </div>
                                case "bitmap":
                                    return <Card key={index} className="inline margin-v-10">
                                        <Card.Header>{i.name}{adornment}</Card.Header>
                                        <Card.Body className={`${!supported ? "disabled" : ""}`}>
                                            {i.bitmap && i.bitmap.map((b: any, bIndex: number) => {
                                                switch (b.type) {
                                                    case "enum":
                                                        return <div key={`${index}:${bIndex}`} className={`col-sm-6 inline margin-v-10 ${(this.state.SupportedAttributes[i.name] === undefined || this.state.SupportedAttributes[i.name].isValid) ? "" : "invalid"}`}>
                                                            {i.writable && b.enum
                                                                ? <TextField size="small" className="writable flex-input" fullWidth={true} select label={b.name} name={b.name} disabled={!supported}
                                                                    value={this.state.SupportedAttributes[i.name]?.Reported[b.name] ?? 0} defaultValue={this.state.SupportedAttributes[i.name]?.Reported[b.name] ?? 0}
                                                                    onChange={this.handleBitmapChange.bind(this, i, false)} variant="outlined" onBlur={this.validateAttr.bind(this, b)} >
                                                                    {b.enum.map((option: any) => {
                                                                        return <MenuItem key={option.name} value={option.name}>
                                                                            {option.name}
                                                                        </MenuItem>
                                                                    })}
                                                                </TextField>
                                                                : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label={b.name} disabled={!supported}
                                                                    value={this.state.SupportedAttributes[i.name]?.Reported[b.name] ?? 0} variant="outlined" />
                                                            }
                                                        </div>
                                                    case "boolean":
                                                        return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                                            <div className={`col-sm-12 no-padding-r ${i.writable ? "writable" : ""}`}>
                                                                <Form.Label column sm="12" className="flex">
                                                                    <div className="check-container">
                                                                        {i.writable
                                                                            ? <Form.Check name={b.name} disabled={!supported} defaultChecked={this.state.SupportedAttributes[i.name]?.Reported[b.name] ?? false} onChange={this.handleBitmapChange.bind(this, i, true)} />
                                                                            : <Form.Check name={b.name} disabled={!supported} readOnly={true} checked={this.state.SupportedAttributes[i.name]?.Reported[b.name] ?? false} />}
                                                                    </div>
                                                                    <div className="word-break">{b.name}</div>
                                                                </Form.Label>
                                                            </div>
                                                        </div>
                                                    default:
                                                        return <div key={`${index}:${bIndex}`} className={`col-sm-6 inline margin-v-10 ${(this.state.SupportedAttributes[i.name] === undefined || this.state.SupportedAttributes[i.name].isValid) ? "" : "invalid"}`}>
                                                            {i.writable && supported
                                                                ? <TextField size="small" className="writable flex-input" fullWidth={true} label={b.name} name={b.name} variant="outlined" type={b.type}
                                                                    defaultValue={this.state.SupportedAttributes[i.name]?.Reported[b.name]} onChange={this.handleBitmapChange.bind(this, i, false)}
                                                                    disabled={!supported} onBlur={this.validateAttr.bind(this, b)} onFocus={(event) => event.target.select()} />
                                                                : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} disabled={!supported} label={b.name} variant="outlined"
                                                                    type="text" value={this.state.SupportedAttributes[i.name]?.Reported[b.name] || ""} />}
                                                        </div>
                                                }
                                            })
                                            }
                                        </Card.Body>
                                    </Card>
                                case "enum":
                                    return <div key={index} className={`col-sm-6 inline margin-v-10 ${(this.state.SupportedAttributes[i.name] === undefined || this.state.SupportedAttributes[i.name].isValid) ? "" : "invalid"}`}>
                                        {i.writable && supported
                                            ? <TextField size="small" className="writable flex-input" fullWidth={true} select label={i.name} name={i.name} value={this.state.SupportedAttributes[i.name]?.Reported ?? ""}
                                                defaultValue={this.state.SupportedAttributes[i.name]?.Reported ?? ""} onChange={this.handleChange} variant="outlined" onBlur={this.validateAttr.bind(this, i)} InputProps={adornment} >
                                                {i.enum.map((option: any) => {
                                                    return <MenuItem key={option.name} value={option.name}>
                                                        {option.name}
                                                    </MenuItem>
                                                })}
                                            </TextField>
                                            : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} disabled={!supported} label={i.name}
                                                value={this.state.SupportedAttributes[i.name]?.Reported} variant="outlined" InputProps={adornment} />
                                        }
                                        {this.getHintText(i)}
                                    </div>
                                default:
                                    return <div key={index} className={`col-sm-6 inline margin-v-10 ${(this.state.SupportedAttributes[i.name] === undefined || this.state.SupportedAttributes[i.name].isValid) ? "" : "invalid"}`}>
                                        {i.writable && supported
                                            ? <TextField size="small" className="writable flex-input" fullWidth={true} label={i.name} name={i.name} variant="outlined" type={i.type}
                                                defaultValue={this.state.SupportedAttributes[i.name]?.Reported} onChange={this.handleChange} onBlur={this.validateAttr.bind(this, i)}
                                                onFocus={(event) => event.target.select()} InputProps={adornment} />
                                            : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} disabled={!supported} label={i.name} variant="outlined" type="text"
                                                value={this.state.SupportedAttributes[i.name]?.Reported || ""} InputProps={adornment} />}
                                        {this.getHintText(i)}
                                    </div>
                            }
                        })}
                    </div>
                </Modal.Body>
                <Modal.Footer>
                    <Tooltip title={`${isWriteAttrSupported ? "" : "Maybe not supported"}`}>
                        <Button variant="outline-primary" className={`${isWriteAttrSupported ? "" : "disabled"}`} onClick={() => this.saveAttr()}>
                            Save
                        </Button>
                    </Tooltip>
                    <Button variant="primary" onClick={() => this.toggleModal(false)}>
                        Cancel
                    </Button>
                </Modal.Footer>
            </Modal >
        );
    }
}

export default ClusterAttr