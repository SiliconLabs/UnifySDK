import * as React from 'react';
import { Card, Form } from 'react-bootstrap';
import { InputAdornment, MenuItem, TextField, Tooltip } from '@mui/material';
import { ClusterTypeAttrs } from '../../../cluster-types/cluster-type-attributes';
import { SceneAttributesProps, SceneAttributesState } from './scene-attributes-types';
import * as MdIcons from 'react-icons/md';
import * as FiIcons from 'react-icons/fi';

class SceneAttributes extends React.Component<SceneAttributesProps, SceneAttributesState> {
    constructor(props: SceneAttributesProps) {
        super(props);
        this.state = {
            Scene: this.props.Scene
        };
    }

    handleChange = (prefixNames: string[], isCheckBox: boolean, isNumber: boolean, event: any) => {
        this.updateAttr(prefixNames, event.target.name, isCheckBox ? event.target.checked : (isNumber ? event.target.valueAsNumber : event.target.value));
    };

    handleBitmapChange(prefixNames: string[], attr: any, isCheckBox: boolean, isNumber: boolean, event: any) {
        let attribute = this.state.Scene.SceneTableExtensions[this.props.Cluster][attr.name];
        attribute[event.target.name] = isCheckBox ? event.target.checked : (isNumber ? event.target.valueAsNumber : event.target.value);
        this.updateAttr(prefixNames, attr.name, attribute);
    }

    updateAttr(prefixNames: string[], name: string, value: any) {
        let attr = this.state.Scene.SceneTableExtensions[this.props.Cluster];
        let attrName = name;
        let attrValue = value;
        if (prefixNames.length > 0) {
            prefixNames.forEach((i: any) => {
                attr = attr[i];
            });
            attr[name] = value;
        }
        else {
            attr[attrName] = attrValue;
        }
        this.setState({ Scene: this.state.Scene });
    }

    removeAttr = (attrName: string) => {
        this.props.RemoveAttribute(this.props.Cluster, attrName);
    }

    getHintText = (item: any) => {
        if (!this.props.IsEdit)
            return;
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

    getAdornment(prefixNames: string[], name: string, type: string, isEdit: boolean) {
        let tooltip = prefixNames.length > 0 || !isEdit ? <></> : <Tooltip title={`Remove Attribute`}>
            <span className="icon">
                <FiIcons.FiTrash onClick={() => this.removeAttr(name)} />
            </span>
        </Tooltip>;
        return type === "boolean" || type === "bitmap"
            ? (prefixNames.length > 0 || !isEdit ? <></> : <div className="adornment">
                {tooltip}
            </div>)
            : {
                endAdornment: <InputAdornment position="end">
                    {tooltip}
                </InputAdornment>
            }
    }

    addArrayItem(attrArray: any[], attrInfo: any) {
        if (attrInfo.type === "struct") {
            let newItem: any = {};
            attrInfo.struct.forEach((i: any) => {
                newItem[i.name] = i.type === "number" ? 0 : "";
            });
            attrArray.push(newItem);
        } else
            attrArray.push(attrInfo.type === "number" ? 0 : "");
        this.setState({ Scene: this.state.Scene });
    }

    removeArrayItem(attr: any[], arrayIndex: number) {
        attr.splice(arrayIndex, 1);
        this.setState({ Scene: this.state.Scene });
    }

    renderField = (i: any, attr: any, isValid: boolean, prefixNames: any[], isEdit: boolean, index: string) => {
        let adornment = this.getAdornment(prefixNames, i.name, i.type, isEdit);
        switch (i.type) {
            case "boolean":
                return <div key={index} className="col-sm-6 inline margin-v-10 attr-item">
                    <div className="col-sm-12 no-padding-r boolean-container">
                        <Form.Label column sm="11" className={isEdit ? "" : "disabled"}>
                            <div className="check-container">
                                {isEdit
                                    ? <Form.Check name={i.name} checked={attr} onChange={this.handleChange.bind(this, prefixNames, true, false)} />
                                    : <Form.Check name={i.name} readOnly={true} checked={attr} />}
                            </div>
                            <span className='word-break'>{i.name}</span></Form.Label>
                        <span className='padding-l-15'>{adornment}</span>
                    </div>
                    {this.getHintText(i)}
                </div>
            case "bitmap":
                return <Card key={index} className="inline margin-v-10">
                    <Card.Header>{i.name}{adornment}</Card.Header>
                    <Card.Body className={isEdit ? "" : "disabled"}>
                        {i.bitmap && i.bitmap.map((b: any, bIndex: number) => {
                            let value;
                            if (attr !== undefined && attr !== null)
                                value = attr[b.name]
                            switch (b.type) {
                                case "enum":
                                    return <div key={`${index}:${bIndex}`} className={`col-sm-6 inline margin-v-10 attr-item ${isValid ? "" : "invalid"}`}>
                                        {isEdit && b.enum
                                            ? <TextField size="small" className="flex-input" fullWidth={true} select={b.enum && b.enum.length > 0} label={b.name} name={b.name}
                                                value={value ?? 0} defaultValue={value ?? 0}
                                                onChange={this.handleBitmapChange.bind(this, prefixNames, i, false, false)} variant="outlined">
                                                {b.enum.map((option: any) => {
                                                    return <MenuItem key={option.name} value={option.name}>
                                                        {option.name}
                                                    </MenuItem>
                                                })}
                                            </TextField>
                                            : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label={b.name} disabled={true}
                                                value={value ?? 0} variant="outlined" />
                                        }
                                    </div>
                                case "boolean":
                                    return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10 attr-item">
                                        <div className="col-sm-12 no-padding-r boolean-container">
                                            <Form.Label column sm="12" className={isEdit ? "flex" : "flex disabled"}>
                                                <div className="check-container">
                                                    {isEdit
                                                        ? <Form.Check name={b.name} checked={value ?? false} onChange={this.handleBitmapChange.bind(this, prefixNames, i, true, false)} />
                                                        : <Form.Check name={b.name} readOnly={true} checked={value ?? false} />}
                                                </div>
                                                <div className="word-break">{b.name}</div>
                                            </Form.Label>
                                        </div>
                                    </div>
                                default:
                                    return <div key={`${index}:${bIndex}`} className={`col-sm-6 inline margin-v-10 attr-item ${isValid ? "" : "invalid"}`}>
                                        {isEdit
                                            ? <TextField size="small" className="flex-input" fullWidth={true} label={b.name} name={b.name} variant="outlined" type={b.type}
                                                defaultValue={value || ""} onChange={this.handleBitmapChange.bind(this, prefixNames, i, false, b.type === "number")}
                                                onFocus={(event) => event.target.select()} />
                                            : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label={b.name} variant="outlined"
                                                type="text" value={value || ""} disabled={true} />}
                                    </div>
                            }
                        })
                        }
                    </Card.Body>
                </Card>
            case "enum":
                return (
                    <div key={index} className={`col-sm-6 inline margin-v-10 attr-item ${isValid ? "" : "invalid"}`}>
                        {isEdit
                            ? <TextField size="small" className="flex-input" fullWidth={true} select={i.enum && i.enum.length > 0} label={i.name} name={i.name} value={attr ?? ""}
                                onChange={this.handleChange.bind(this.props, prefixNames, false, false)} variant="outlined" InputProps={adornment} >
                                {i.enum.map((option: any) => {
                                    return <MenuItem key={option.name} value={option.name}>
                                        {option.name}
                                    </MenuItem>
                                })}
                            </TextField>
                            : <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label={i.name}
                                value={attr} variant="outlined" InputProps={adornment} disabled={true} />
                        }
                        {this.getHintText(i)}
                    </div>
                )
            case "struct":
                return <Card key={index} className="inline margin-v-10">
                    <Card.Header>{i.name} {i.isArray
                        ? <span className="pointer icon">
                            {isEdit
                                ? <Tooltip title={`Add Item`} className="float-right">
                                    <span className="pointer icon">
                                        <FiIcons.FiPlus onClick={this.addArrayItem.bind(this, attr, i)} />
                                    </span>
                                </Tooltip>
                                : null
                            }
                            <Tooltip className="float-right" title={isEdit ? "Remove Attribute" : ""} hidden={!isEdit}>
                                <span className="icon">
                                    <MdIcons.MdRefresh onClick={() => this.removeAttr(i.name)} />
                                </span>
                            </Tooltip>
                        </span>
                        : <></>}
                    </Card.Header>
                    <Card.Body className='col-sm-12 no-padding'>
                        {i.isArray ?
                            (attr?.length === undefined || attr.length === 0
                                ? <></>
                                : attr.map((arrayItem: any, arrayIndex: number) => {
                                    return <div key={`${index}-${arrayIndex}`} className='col-sm-6 inline'>
                                        <Card className="margin-v-10 flex-input">
                                            <Card.Header className='padding-2'>[{arrayIndex}]
                                                {isEdit
                                                    ? <span className="pointer icon"><FiIcons.FiMinus className="float-right" onClick={this.removeArrayItem.bind(this, attr, arrayIndex)} /></span>
                                                    : null}
                                            </Card.Header>
                                            <Card.Body className="col-sm-12 no-padding">
                                                {
                                                    i.struct.map((field: any, sIndex: number) => {
                                                        return this.renderField(field, arrayItem[field.name], isValid, prefixNames.concat(i.name).concat(arrayIndex), isEdit, `${index}-${arrayIndex}-${sIndex}`);
                                                    })
                                                }
                                            </Card.Body>
                                        </Card>
                                    </div>
                                })
                            )
                            : i.struct.map((field: any, sIndex: number) => {
                                return this.renderField(field, attr && attr[field.name], isValid, prefixNames.concat(i.name), isEdit, `${index}-${sIndex}`);
                            })
                        }
                    </Card.Body>
                </Card>
            default:
                return isEdit
                    ? (i.isArray ?
                        (attr?.length === undefined || attr.length === 0
                            ? <div key={index}></div>
                            : <Card key={index} className="inline margin-v-10">
                                <Card.Header>{i.name}
                                    {isEdit
                                        ? <Tooltip title={`Add Item`} className="float-right">
                                            <span className="pointer icon">
                                                <FiIcons.FiPlus onClick={this.addArrayItem.bind(this, attr, i)} />
                                            </span>
                                        </Tooltip>
                                        : null}
                                    <Tooltip className="float-right" title={isEdit ? "Remove Attribute" : ""} hidden={!isEdit}>
                                        <span className="icon">
                                            <MdIcons.MdRefresh onClick={() => this.removeAttr(i.name)} />
                                        </span>
                                    </Tooltip>
                                </Card.Header>
                                <div className='col-sm-12 no-padding'>
                                    {attr.map((arrayItem: any, arrayIndex: number) => {
                                        return <Card.Body key={`${index}-${arrayIndex}`} className="col-sm-6 inline no-padding-v margin-v-10">
                                            <TextField size="small" className="flex-input" label={`[${arrayIndex}]`} name={arrayIndex.toString()} variant="outlined" type={i.type}
                                                value={arrayItem} onChange={this.handleChange.bind(this, prefixNames.concat(i.name), false, i.type === "number")}
                                                onFocus={(event) => event.target.select()} InputProps={{
                                                    endAdornment: <InputAdornment position="end">
                                                        {isEdit
                                                            ? <Tooltip title={`Remove Item`}>
                                                                <span className="pointer icon">
                                                                    <FiIcons.FiMinus onClick={this.removeArrayItem.bind(this, attr, arrayIndex)} />
                                                                </span>
                                                            </Tooltip>
                                                            : null}
                                                    </InputAdornment>
                                                }} />
                                        </Card.Body>
                                    })}
                                </div>
                            </Card>
                        )
                        : <div key={index} className={`col-sm-6 inline margin-v-10 attr-item ${isValid ? "" : "invalid"}`}>
                            <TextField size="small" className="flex-input" fullWidth={true} label={i.name} name={i.name} variant="outlined" type={i.type}
                                value={attr || ""} onChange={this.handleChange.bind(this, prefixNames, false, i.type === "number")}
                                onFocus={(event) => event.target.select()} InputProps={adornment} />
                            {this.getHintText(i)}
                        </div>
                    )
                    : <div key={index} className={`col-sm-6 inline margin-v-10 attr-item ${isValid ? "" : "invalid"}`}>
                        <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} disabled={true} label={i.name} variant="outlined" type="text"
                            value={i.isArray && attr !== undefined ? attr.join("; ") : (attr || "")} InputProps={adornment} />
                        {this.getHintText(i)}
                    </div>
        }
    }

    render() {
        let attrs = Object.keys(this.state.Scene.SceneTableExtensions[this.props.Cluster]) || [];
        return (
            ClusterTypeAttrs[this.props.Cluster]?.server?.attributes.map((i: any, index: number) => {
                if (attrs.indexOf(i.name) > -1) {
                    return this.renderField(i, this.state.Scene.SceneTableExtensions[this.props.Cluster][i.name], true, [], this.props.IsEdit, index.toString())
                }
            })
        );
    }
}

export default SceneAttributes