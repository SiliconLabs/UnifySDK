import { MenuItem, TextField } from '@material-ui/core';
import * as React from 'react';
import { Card, Form } from 'react-bootstrap';
import { CommandAttrsState } from './command-attrs-types';

class CommandAttrs extends React.Component<{}, CommandAttrsState> {
    constructor(props: {}) {
        super(props);
        this.state = {
            Payload: {},
            Command: {}
        };
    }

    updateState(command: any) {
        this.setState({
            Payload: {},
            Command: {}
        }, () => this.setState({ Payload: this.getPayload(command), Command: command }));
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
            }
            else
                payload[i.name] = i.default !== undefined ? i.default : (i.type === "boolean" ? false : (i.type === "text" ? "" : 0));
        });
        return payload;
    }

    handleChange(isCheckBox: boolean, event: any) {
        let tempData = { ...this.state.Payload };
        tempData[event.target.name] = isCheckBox ? event.target.checked : event.target.value;
        this.setState({ Payload: tempData });
    }

    handleBitmapChange(attr: any, isCheckBox: boolean, event: any) {
        let tempData = this.state.Payload[attr.name];
        tempData[event.target.name] = isCheckBox ? event.target.checked : event.target.value;
        this.setState({ Payload: this.state.Payload });
    }

    render() {
        return (<>
            {this.state.Command && this.state.Command.fields && this.state.Command.fields.map((i: any, index: number) => {
                if (i.enum && i.enum.length) {
                    return (
                        <div key={index} className="col-sm-6 inline margin-v-10">
                            <TextField size="small" className="flex-input" fullWidth={true} select label={i.name} name={i.name} value={this.state.Payload[i.name]} onChange={this.handleChange.bind(this, false)} variant="outlined">
                                {i.enum.map((j: any, ind: number) => {
                                    return <MenuItem key={ind} value={j.name}>
                                        {j.name}
                                    </MenuItem>
                                })}
                            </TextField>
                        </div>
                    )
                } else if (i.bitmap && i.bitmap.length) {
                    return <Card key={index} className="inline margin-v-10">
                        <Card.Header>{i.name}</Card.Header>
                        <Card.Body>
                            {i.bitmap.map((b: any, bIndex: number) => {
                                switch (b.type) {
                                    case "enum":
                                        return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                            <TextField size="small" className="flex-input" fullWidth={true} select label={b.name} name={b.name} value={this.state.Payload[i.name][b.name] ?? 0} defaultValue={this.state.Payload[i.name][b.name] ?? 0} onChange={this.handleBitmapChange.bind(this, i, false)} variant="outlined">
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
                                                        <Form.Check name={b.name} defaultChecked={this.state.Payload[i.name][b.name] ?? false} onChange={this.handleBitmapChange.bind(this, i, true)} />
                                                    </div>
                                                    {b.name}</Form.Label>
                                            </div>
                                        </div>
                                    default:
                                        return <div key={`${index}:${bIndex}`} className="col-sm-6 inline margin-v-10">
                                            <TextField size="small" className="flex-input" fullWidth={true} label={b.name} name={b.name} variant="outlined" type={b.type} defaultValue={this.state.Payload[i.name][b.name]} onChange={this.handleBitmapChange.bind(this, i, false)} onFocus={(event) => event.target.select()} />
                                        </div>
                                }
                            })
                            }
                        </Card.Body>
                    </Card>
                } else
                    switch (i.type) {
                        case "boolean":
                            return (
                                <div key={index} className="col-sm-6 inline margin-v-10">
                                    <div className="col-sm-12">
                                        <Form.Label column sm="11">
                                            <div className="check-container">
                                                <Form.Check name={i.name} defaultChecked={this.state.Payload[i.name] ?? false} checked={this.state.Payload[i.name]} onChange={this.handleChange.bind(this, true)} />
                                            </div>
                                            {i.name}</Form.Label>
                                    </div>
                                </div>
                            )
                        default:
                            return (
                                <div key={index} className="col-sm-6 inline margin-v-10">
                                    <TextField size="small" className="flex-input" fullWidth={true} label={i.name} name={i.name} variant="outlined" type={i.type} defaultValue={this.state.Payload[i.name]} onChange={this.handleChange.bind(this, false)} onFocus={(event) => event.target.select()} />
                                </div>
                            )
                    }
            })}
        </>);
    }
}

export default CommandAttrs