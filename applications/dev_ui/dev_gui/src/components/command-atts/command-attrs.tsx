import * as React from 'react';
import { Card, Form } from 'react-bootstrap';
import { CommandAttrsState } from './command-attrs-types';
import * as FiIcons from 'react-icons/fi';
import { InputAdornment, MenuItem, TextField, Tooltip } from '@mui/material';

class CommandAttrs extends React.Component<{}, CommandAttrsState> {
    constructor(props: {}) {
        super(props);
        this.state = {
            Payload: {},
            Command: {},
            ReadOnly: false
        };
    }

    updateState(command: any, payload: any = null, readOnly: boolean = false) {
        this.setState({
            Payload: {},
            Command: {}
        }, () => this.setState({ Payload: payload || this.getPayload(command), Command: command, ReadOnly: readOnly }));
    }

    getPayload(command: any) {
        let payload: any = {};
        command && command.fields && command.fields.forEach((i: any) => {
            if (i.enum && i.enum.length)
                payload[i.name] = i.enum[0].name;
            else if (i.bitmap && i.bitmap.length) {
                payload[i.name] = {};
                i.bitmap.forEach((b: any) => {
                    if (b.enum && b.enum.length)
                        payload[i.name][b.name] = b.enum[0].name;
                    else
                        payload[i.name][b.name] = b.default !== undefined ? b.default : (b.type === "boolean" ? false : (b.type === "text" ? "" : 0));
                });
            } else if (i.struct && i.struct.length) {
                let item: any = {}
                i.struct.forEach((s: any) => {
                    item[s.name] = s.type === "boolean" ? false : (s.type === "number" ? 0 : "");
                });
                payload[i.name] = i.isArray ? [] : item;
            } else
                payload[i.name] = i.isArray ? [] : (i.default !== undefined ? i.default : (i.type === "boolean" ? false : (i.type === "text" ? "" : 0)));
        });
        return payload;
    }

    handleChange(prefixNames: string[], isCheckBox: boolean, isNumber: boolean, event: any) {
        let tempData = this.state.Payload;
        prefixNames.forEach((i: any) => tempData = tempData[i]);
        tempData[event.target.name] = isCheckBox ? event.target.checked : (isNumber ? (isNaN(event.target.valueAsNumber) ? null : event.target.valueAsNumber) : event.target.value);
        this.setState({ Payload: this.state.Payload });
    }

    addArrayItem(attrArray: any[], attrInfo: any) {
        if (attrInfo.type === "struct") {
            let newItem: any = {};
            attrInfo.struct.forEach((i: any) => {
                if (i.isArray)
                    newItem[i.name] = [];
                else
                    newItem[i.name] = i.type === "number" ? 0 : "";
            });
            attrArray.push(newItem);
        } else
            attrArray.push(attrInfo.type === "number" ? 0 : "");
        this.setState({ Payload: this.state.Payload });
    }

    removeArrayItem(attr: any[], arrayIndex: number) {
        attr.splice(arrayIndex, 1);
        this.setState({ Payload: this.state.Payload });
    }

    getValue = (type: string, value: any) => {
        if (type === "number" && value !== undefined && value !== null)
            return Number(Math.round((value + "e+" + 3) as unknown as number) + "e-" + 3)
        else return value || "";
    }

    renderField = (item: any, payload: any, prefixNames: any[], index: any) => {
        switch (item.type) {
            case "enum":
                if (item.enum && item.enum.length) {
                    return (
                        <div key={index} className="col-sm-6 inline margin-v-10">
                            <TextField size="small" className="flex-input" fullWidth={true} select label={item.name} name={item.name}
                                value={payload} onChange={this.handleChange.bind(this, prefixNames, false, false)} variant="outlined"
                                inputProps={
                                    { readOnly: this.state.ReadOnly }
                                }>
                                {item.enum.map((j: any, ind: number) => {
                                    return <MenuItem key={ind} value={j.name}>
                                        {j.name}
                                    </MenuItem>
                                })}
                            </TextField>
                        </div>
                    )
                }
                break;
            case "bitmap":
                if (item.bitmap && item.bitmap.length) {
                    return <Card key={index} className="inline margin-v-10">
                        <Card.Header>{item.name}</Card.Header>
                        <Card.Body>
                            {item.bitmap.map((b: any, bIndex: number) => {
                                switch (b.type) {
                                    case "enum":
                                        return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                            <TextField size="small" className="flex-input" fullWidth={true} select label={b.name} name={b.name} value={payload[b.name] ?? 0}
                                                defaultValue={payload[b.name] ?? 0} onChange={this.handleChange.bind(this, prefixNames.concat(item.name), false, false)} variant="outlined" inputProps={
                                                    { readOnly: this.state.ReadOnly }
                                                }>
                                                {b.enum.map((option: any) => {
                                                    return <MenuItem key={option.name} value={option.name}>
                                                        {option.name}
                                                    </MenuItem>
                                                })}
                                            </TextField>
                                        </div>
                                    case "boolean":
                                        return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                            <div className="col-sm-12">
                                                <Form.Label column sm="11">
                                                    <div className="check-container">
                                                        {this.state.ReadOnly
                                                            ? <Form.Check readOnly checked={payload[b.name] ?? false} />
                                                            : <Form.Check name={b.name} defaultChecked={payload[b.name] ?? false} onChange={this.handleChange.bind(this, prefixNames.concat(item.name), true, false)} />}
                                                    </div>
                                                    {b.name}</Form.Label>
                                            </div>
                                        </div>
                                    default:
                                        return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                            <TextField size="small" className="flex-input" fullWidth={true} label={b.name} name={b.name} variant="outlined" type={b.type}
                                                defaultValue={this.getValue(b.type, payload[b.name])} onChange={this.handleChange.bind(this, prefixNames.concat(item.name), false, b.type === "number")} onFocus={(event: any) => event.target.select()}
                                                inputProps={
                                                    { readOnly: this.state.ReadOnly }
                                                } />
                                        </div>
                                }
                            })
                            }
                        </Card.Body>
                    </Card>
                }
                break;
            case "struct":
                if (item.struct && item.struct.length) {
                    return <Card key={index} className="inline margin-v-10">
                        <Card.Header>{item.name}
                            {item.isArray
                                ? <span className="pointer icon">
                                    <Tooltip title={`Add Item`} className="float-right">
                                        <span className="pointer icon">
                                            <FiIcons.FiPlus onClick={this.addArrayItem.bind(this, payload, item)} />
                                        </span>
                                    </Tooltip>
                                </span>
                                : <></>}
                        </Card.Header>
                        <Card.Body className='col-sm-12 no-padding'>
                            {item.isArray
                                ? payload.map((arrayItem: any, arrayIndex: number) => {
                                    return <div key={`${index}-${arrayIndex}`} className='col-sm-6 inline'>
                                        <Card className="margin-v-10 flex-input">
                                            <Card.Header className='padding-2'>[{arrayIndex}]<span className="pointer icon"><FiIcons.FiMinus className="float-right" onClick={this.removeArrayItem.bind(this, payload, arrayIndex)} /></span></Card.Header>
                                            <Card.Body className="col-sm-12 no-padding">
                                                {
                                                    item.struct.map((field: any, sIndex: number) => {
                                                        return this.renderField(field, arrayItem[field.name], prefixNames.concat(item.name).concat(arrayIndex), `${index}-${arrayIndex}-${sIndex}`);
                                                    })
                                                }
                                            </Card.Body>
                                        </Card>
                                    </div>
                                })
                                : item.struct.map((field: any, sIndex: number) => {
                                    return this.renderField(field, payload[field.name], prefixNames.concat(item.name), `${index}-${sIndex}`);
                                })
                            }
                        </Card.Body>
                    </Card>

                }
                break;
            case "boolean":
                return (
                    <div key={index} className="col-sm-6 inline margin-v-10">
                        <div className="col-sm-12">
                            <Form.Label column sm="11">
                                <div className="check-container">
                                    {this.state.ReadOnly
                                        ? <Form.Check readOnly checked={payload} />
                                        : <Form.Check name={item.name} defaultChecked={payload ?? false} checked={payload} onChange={this.handleChange.bind(this, prefixNames, true, false)} />}
                                </div>
                                {item.name}</Form.Label>
                        </div>
                    </div>
                )
            default:
                return (item.isArray
                    ? <Card key={index} className="inline margin-v-10">
                        <Card.Header>{item.name}
                            <Tooltip title={`Add Item`} className="float-right">
                                <span className="pointer icon">
                                    <FiIcons.FiPlus onClick={this.addArrayItem.bind(this, payload, item)} />
                                </span>
                            </Tooltip>
                        </Card.Header>
                        <div className='col-sm-12 no-padding'>
                            {payload.map((arrayItem: any, arrayIndex: number) => {
                                return <Card.Body key={`${index}-${arrayIndex}`} className="col-sm-6 inline no-padding-v margin-v-10">
                                    <TextField size="small" className="flex-input" label={`[${arrayIndex}]`} name={arrayIndex.toString()} variant="outlined" type={item.type}
                                        value={this.getValue(item.type, arrayItem)} onChange={this.handleChange.bind(this, prefixNames.concat(item.name), false, item.type === "number")}
                                        onFocus={(event: any) => event.target.select()} InputProps={{
                                            endAdornment: <InputAdornment position="end">
                                                <Tooltip title={`Remove Item`}>
                                                    <span className="pointer icon">
                                                        <FiIcons.FiMinus onClick={this.removeArrayItem.bind(this, payload, arrayIndex)} />
                                                    </span>
                                                </Tooltip>
                                            </InputAdornment>
                                        }} />
                                </Card.Body>
                            })}
                        </div>
                    </Card>
                    : <div key={index} className={`col-sm-6 inline margin-v-10`}>
                        <TextField size="small" className="flex-input" fullWidth={true} label={item.name} name={item.name} variant="outlined" type={item.type}
                            value={this.getValue(item.type, payload)} onChange={this.handleChange.bind(this, prefixNames, false, item.type === "number")} onFocus={(event: any) => event.target.select()} inputProps={
                                { readOnly: this.state.ReadOnly }
                            } />
                    </div>
                )
        }
    }

    render() {
        return (<>
            {this.state.Command && this.state.Command.fields && this.state.Command.fields.map((i: any, index: number) => this.renderField(i, this.state.Payload[i.name], [], index))}
        </>);
    }
}

export default CommandAttrs