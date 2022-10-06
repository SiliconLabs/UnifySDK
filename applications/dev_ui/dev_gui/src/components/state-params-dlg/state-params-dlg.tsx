import { TextField } from '@mui/material';
import * as React from 'react';
import { Button, Form, Modal } from 'react-bootstrap';
import { ClusterTypeAttrs } from '../../cluster-types/cluster-type-attributes';
import { StateParams, StateParamsProps } from './state-params-dlg-types';


class StateParamsDlg extends React.Component<StateParamsProps, StateParams> {
    constructor(props: StateParamsProps) {
        super(props);
        this.state = {
            ShowModal: false,
            StateParameters: {},
            State: "",
            Node: {}
        };
    }

    updateState(showModal: boolean, node: any = {}) {
        this.setState({ ShowModal: showModal, StateParameters: {}, Node: node });
    }

    send = () => {
        this.props.SocketServer.send(JSON.stringify({
            type: "run-state-command",
            data: {
                Unid: this.state.Node.Unid,
                Payload: {
                    State: this.state.Node.NetworkManagementState.State,
                    StateParameters: this.state.StateParameters
                }
            }
        }));
        this.setState({ ShowModal: false });
    }

    getJSONValue = (target: any) => {
        let value = target.value !== undefined && target.value.trim("\"")
        try {
            return JSON.parse(value);
        } catch {
            return value;
        }
    }

    handleChange = (isCheckBox: boolean, isNumber: boolean, isJSON: boolean, event: any) => {
        let supAttr = this.state.StateParameters;
        let value = isCheckBox ? event.target.checked : (isNumber ? event.target.valueAsNumber : (isJSON ? this.getJSONValue(event.target) : event.target.value));
        supAttr[event.target.name] = value;
        this.setState({ StateParameters: supAttr });
    };

    render() {
        let attrs = ClusterTypeAttrs.NetworkManagement?.server?.attributes?.find((i: any) => i.name === "StateParameters")?.struct;
        return (
            <Modal show={this.state.ShowModal} onHide={() => this.updateState(false)} >
                <Modal.Header>
                    <Modal.Title>
                        Setup State Parameters
                    </Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    {this.state.Node.NetworkManagementState?.RequestedStateParameters && this.state.Node.NetworkManagementState?.RequestedStateParameters.map((param: any, index: number) => {
                        let attr = attrs && attrs.find((i: any) => i.name === param);
                        if (attr) {
                            if (attr.type === "boolean") {
                                return <div key={index} className="col-sm-12 inline margin-v-10">
                                    <Form.Label column sm="11">
                                        <div className="check-container">
                                            <Form.Check name={attr.name} onChange={this.handleChange.bind(this, true, false, false)} />
                                        </div>
                                        {attr.name}</Form.Label>
                                </div>
                            } else {
                                return <div key={index} className="col-sm-12 inline margin-v-10">
                                    <TextField size="small" name={attr.name} className="flex-input" fullWidth={true} label={attr.name} variant="outlined" type={attr.type} onChange={this.handleChange.bind(this, false, attr.type === "number", false)} />
                                </div>
                            }
                        }
                        else {
                            return <div key={index} className="col-sm-12 inline margin-v-10">
                                <TextField size="small" name={param} className="flex-input" fullWidth={true} label={param+" [JSON format]"} variant="outlined" type="text" onChange={this.handleChange.bind(this, false, false, true)} />
                            </div>

                        }
                    })}
                </Modal.Body>
                <Modal.Footer>
                    <Button variant="outline-primary" onClick={this.send}>Send</Button>
                    <Button variant="primary" onClick={() => this.updateState(false)}>Cancel</Button>
                </Modal.Footer>
            </Modal>
        );
    }
}

export default StateParamsDlg