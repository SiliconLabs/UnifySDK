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

    handleChange = (prefixNames: string[], isCheckBox: boolean, isNumber: boolean, event: any) => {
        this.updateAttr(prefixNames, event.target.name, isCheckBox ? event.target.checked : (isNumber ? event.target.valueAsNumber : event.target.value));
    };

    handleBitmapChange(prefixNames: string[], attr: any, isCheckBox: boolean, isNumber: boolean, event: any) {
        let sup = this.state.SupportedAttributes[attr.name];
        sup.Reported[event.target.name] = isCheckBox ? event.target.checked : (isNumber ? event.target.valueAsNumber : event.target.value);
        this.updateAttr(prefixNames, attr.name, sup.Reported);
    }

    updateAttr(prefixNames: string[], name: string, value: any) {
        let supAttr = this.state.SupportedAttributes;
        let attrName = name;
        let attrValue = value;
        if (prefixNames.length > 0) {
            prefixNames.forEach((i: any) => {
                supAttr = supAttr[i];
                if (supAttr.Reported !== undefined) {
                    supAttr = supAttr.Reported;
                    attrName = i;
                    attrValue = supAttr;
                }
            });
            supAttr[name] = value;
        }
        else {
            supAttr[attrName].Reported = attrValue;
        }
        this.changedAttrList.set(attrName, attrValue);
        this.setState({ SupportedAttributes: this.state.SupportedAttributes });
    }


    addArrayItem(prefixNames: any[], attrArray: any[], attrInfo: any) {
        if (attrInfo.type === "struct") {
            let newItem: any = {};
            attrInfo.struct.forEach((i: any) => {
                newItem[i.name] = i.type === "number" ? 0 : "";
            });
            attrArray.push(newItem);
        } else
            attrArray.push(attrInfo.type === "number" ? 0 : "");
        this.changedAttrList.set(prefixNames[0], this.state.SupportedAttributes[prefixNames[0]].Reported);
        this.setState({ SupportedAttributes: this.state.SupportedAttributes });
    }

    removeArrayItem(prefixNames: any[], attr: any[], arrayIndex: number) {
        attr.splice(arrayIndex, 1);
        this.changedAttrList.set(prefixNames[0], this.state.SupportedAttributes[prefixNames[0]].Reported);
        this.setState({ SupportedAttributes: this.state.SupportedAttributes });
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

    validateAttr(item: any, prefixNames: string[]) {
        let supAttr = this.state.SupportedAttributes;
        let name = prefixNames.length ? prefixNames[0] : item.name;
        supAttr[name].isValid = this.validate(name);
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
        return hints.length ? <Form.Text className="hint">{hints.join("; ")}</Form.Text> : null;
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

    getAdornment(prefixNames: string[], name: string, type: string, isSupported: boolean) {
        let tooltip = prefixNames.length > 0 ? <></> : <Tooltip title={`Force Read Attribute ${isSupported ? "" : "(maybe not supported)"}`}>
            <span className={`${isSupported ? "" : "disabled pointer"} icon`}>
                <MdIcons.MdRefresh onClick={() => this.forceRead(name)} />
            </span>
        </Tooltip>;
        return type === "boolean" || type === "bitmap"
            ? (prefixNames.length > 0 ? <></> : <div className="adornment">
                {tooltip}
            </div>)
            : {
                endAdornment: <InputAdornment position="end">
                    {tooltip}
                </InputAdornment>
            }
    }

    renderField = (i: any, attr: any, isValid: boolean, isForceReadAttrSupported: boolean, prefixNames: any[], supported: boolean, writable: boolean, index: string) => {
        let adornment = this.getAdornment(prefixNames, i.name, i.type, isForceReadAttrSupported);
        if (this.state.HideUnSup && !supported)
            return <span key={index} hidden={true}></span>;
        switch (i.type) {
            case "boolean":
                return <div key={index} className="col-sm-6 inline margin-v-10">
                    <div className={`col-sm-12 no-padding-r ${writable ? "writable" : ""}`}>
                        <Form.Label column sm="11" className={supported ? "" : "disabled"}>
                            <div className="check-container">
                                {writable && supported
                                    ? <Form.Check name={i.name} disabled={!supported} defaultChecked={attr} onChange={this.handleChange.bind(this, prefixNames, true, false)} />
                                    : <Form.Check name={i.name} readOnly={true} disabled={!supported} checked={attr} />}
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
                            let value;
                            if (attr !== undefined && attr.Reported !== null)
                                value = attr[b.name]
                            switch (b.type) {
                                case "enum":
                                    return <div key={`${index}:${bIndex}`} className={`col-sm-6 inline margin-v-10 ${isValid ? "" : "invalid"}`}>
                                        {writable && b.enum
                                            ? <TextField size="small" className="writable flex-input" fullWidth={true} select label={b.name} name={b.name} disabled={!supported}
                                                value={value ?? 0} defaultValue={value ?? 0}
                                                onChange={this.handleBitmapChange.bind(this, prefixNames, i, false, false)} variant="outlined" onBlur={this.validateAttr.bind(this, b, prefixNames.concat(i.name))} >
                                                {b.enum.map((option: any) => {
                                                    return <MenuItem key={option.name} value={option.name}>
                                                        {option.name}
                                                    </MenuItem>
                                                })}
                                            </TextField>
                                            : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label={b.name} disabled={!supported}
                                                value={value ?? 0} variant="outlined" />
                                        }
                                    </div>
                                case "boolean":
                                    return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                        <div className={`col-sm-12 no-padding-r ${writable ? "writable" : ""}`}>
                                            <Form.Label column sm="12" className="flex">
                                                <div className="check-container">
                                                    {writable
                                                        ? <Form.Check name={b.name} disabled={!supported} defaultChecked={value ?? false} onChange={this.handleBitmapChange.bind(this, prefixNames, i, true, false)} />
                                                        : <Form.Check name={b.name} disabled={!supported} readOnly={true} checked={value ?? false} />}
                                                </div>
                                                <div className="word-break">{b.name}</div>
                                            </Form.Label>
                                        </div>
                                    </div>
                                default:
                                    return <div key={`${index}:${bIndex}`} className={`col-sm-6 inline margin-v-10 ${isValid ? "" : "invalid"}`}>
                                        {writable && supported
                                            ? <TextField size="small" className="writable flex-input" fullWidth={true} label={b.name} name={b.name} variant="outlined" type={b.type}
                                                defaultValue={value || ""} onChange={this.handleBitmapChange.bind(this, prefixNames, i, false, b.type === "number")}
                                                disabled={!supported} onBlur={this.validateAttr.bind(this, b, prefixNames.concat(i.name))} onFocus={(event) => event.target.select()} />
                                            : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} disabled={!supported} label={b.name} variant="outlined"
                                                type="text" value={value || ""} />}
                                    </div>
                            }
                        })
                        }
                    </Card.Body>
                </Card>
            case "enum":
                return <div key={index} className={`col-sm-6 inline margin-v-10 ${isValid ? "" : "invalid"}`}>
                    {writable && supported
                        ? <TextField size="small" className="writable flex-input" fullWidth={true} select label={i.name} name={i.name} value={attr ?? ""}
                            defaultValue={attr ?? ""} onChange={this.handleChange.bind(this.props, prefixNames, false, false)} variant="outlined" onBlur={this.validateAttr.bind(this, i, prefixNames)} InputProps={adornment} >
                            {i.enum.map((option: any) => {
                                return <MenuItem key={option.name} value={option.name}>
                                    {option.name}
                                </MenuItem>
                            })}
                        </TextField>
                        : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} disabled={!supported} label={i.name}
                            value={attr} variant="outlined" InputProps={adornment} />
                    }
                    {this.getHintText(i)}
                </div>
            case "struct":
                return <Card key={index} className="inline margin-v-10">
                    <Card.Header>{i.name} {i.isArray
                        ? <span className="pointer icon">
                            <Tooltip title={`Add Item`} className="float-right">
                                <span className="pointer icon">
                                    <FiIcons.FiPlus onClick={this.addArrayItem.bind(this, prefixNames.concat(i.name), attr, i)} />
                                </span>
                            </Tooltip>
                            <Tooltip className="float-right" title={`Force Read Attribute ${isForceReadAttrSupported ? "" : "(maybe not supported)"}`}>
                                <span className={`${isForceReadAttrSupported ? "" : "disabled pointer"} icon`}>
                                    <MdIcons.MdRefresh onClick={() => this.forceRead(i.name)} />
                                </span>
                            </Tooltip>
                        </span>
                        : <></>}
                    </Card.Header>
                    <Card.Body className='col-sm-12 no-padding'>
                        {i.isArray ?
                            (attr.length === undefined || attr.length === 0
                                ? <></>
                                : attr.map((arrayItem: any, arrayIndex: number) => {
                                    return <div key={`${index}-${arrayIndex}`} className='col-sm-6 inline'>
                                        <Card className="margin-v-10 flex-input">
                                            <Card.Header className='padding-2'>[{arrayIndex}]<span className="pointer icon"><FiIcons.FiMinus className="float-right" onClick={this.removeArrayItem.bind(this, prefixNames.concat(i.name), attr, arrayIndex)} /></span></Card.Header>
                                            <Card.Body className="col-sm-12 no-padding">
                                                {
                                                    i.struct.map((field: any, sIndex: number) => {
                                                        return this.renderField(field, arrayItem[field.name], isValid, isForceReadAttrSupported, prefixNames.concat(i.name).concat(arrayIndex), supported, writable, `${index}-${arrayIndex}-${sIndex}`);
                                                    })
                                                }
                                            </Card.Body>
                                        </Card>
                                    </div>
                                })
                            )
                            : i.struct.map((field: any, sIndex: number) => {
                                return this.renderField(field, attr[field.name], isValid, isForceReadAttrSupported, prefixNames.concat(i.name), supported, writable, `${index}-${sIndex}`);
                            })
                        }
                    </Card.Body>
                </Card>
            default:
                return writable && supported
                    ? (i.isArray ?
                        (attr.length === undefined || attr.length === 0
                            ? <div key={index}></div>
                            : <Card key={index} className="inline margin-v-10">
                                <Card.Header>{i.name}
                                    <Tooltip title={`Add Item`} className="float-right">
                                        <span className="pointer icon">
                                            <FiIcons.FiPlus onClick={this.addArrayItem.bind(this, prefixNames.concat(i.name), attr, i)} />
                                        </span>
                                    </Tooltip>
                                    <Tooltip className="float-right" title={`Force Read Attribute ${isForceReadAttrSupported ? "" : "(maybe not supported)"}`}>
                                        <span className={`${isForceReadAttrSupported ? "" : "disabled pointer"} icon`}>
                                            <MdIcons.MdRefresh onClick={() => this.forceRead(i.name)} />
                                        </span>
                                    </Tooltip>
                                </Card.Header>
                                <div className='col-sm-12 no-padding'>
                                    {attr.map((arrayItem: any, arrayIndex: number) => {
                                        return <Card.Body key={`${index}-${arrayIndex}`} className="col-sm-6 inline no-padding-v margin-v-10">
                                            <TextField size="small" className="writable flex-input" label={`[${arrayIndex}]`} name={arrayIndex.toString()} variant="outlined" type={i.type}
                                                value={arrayItem} onChange={this.handleChange.bind(this, prefixNames.concat(i.name), false, i.type === "number")} onBlur={this.validateAttr.bind(this, i, prefixNames)}
                                                onFocus={(event) => event.target.select()} InputProps={{
                                                    endAdornment: <InputAdornment position="end">
                                                        <Tooltip title={`Remove Item`}>
                                                            <span className="pointer icon">
                                                                <FiIcons.FiMinus onClick={this.removeArrayItem.bind(this, prefixNames.concat(i.name), attr, arrayIndex)} />
                                                            </span>
                                                        </Tooltip>
                                                    </InputAdornment>
                                                }} />
                                        </Card.Body>
                                    })}
                                </div>
                            </Card>
                        )
                        : <div key={index} className={`col-sm-6 inline margin-v-10 ${isValid ? "" : "invalid"}`}>
                            <TextField size="small" className="writable flex-input" fullWidth={true} label={i.name} name={i.name} variant="outlined" type={i.type}
                                value={attr || ""} onChange={this.handleChange.bind(this, prefixNames, false, i.type === "number")} onBlur={this.validateAttr.bind(this, i, prefixNames)}
                                onFocus={(event) => event.target.select()} InputProps={adornment} />
                            {this.getHintText(i)}
                        </div>
                    )
                    : <div key={index} className={`col-sm-6 inline margin-v-10 ${isValid ? "" : "invalid"}`}>
                        <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} disabled={!supported} label={i.name} variant="outlined" type="text"
                            value={i.isArray ? attr.join("; ") : (attr || "")} InputProps={adornment} />
                        {this.getHintText(i)}
                    </div>
        }
    }

    render() {
        let keys = (this.state.SupportedAttributes && Object.keys(this.state.SupportedAttributes)) || [];
        let isForceReadAttrSupported = this.state.SupportedCommands !== undefined && this.state.SupportedCommands.indexOf("ForceReadAttributes") > -1;
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
                            let isValid = this.state.SupportedAttributes[i.name] === undefined || this.state.SupportedAttributes[i.name].isValid === true;
                            return this.renderField(i, this.state.SupportedAttributes[i.name]?.Reported, isValid, isForceReadAttrSupported,
                                [], supported, i.writable, index.toString())
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