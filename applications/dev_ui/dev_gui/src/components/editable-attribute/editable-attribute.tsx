import * as React from 'react';
import { Button, Form, Modal } from 'react-bootstrap';
import { TextField } from '@material-ui/core';
import { EditableAttributeProps, EditableAttributeState } from './editable-attribute-types';
import Autocomplete from '@mui/material/Autocomplete';


class EditableAttribute extends React.Component<EditableAttributeProps, EditableAttributeState> {
    constructor(props: EditableAttributeProps) {
        super(props);
        this.state = {
            ShowModal: false,
            Value: this.props.Cluster?.Attributes?.[this.props.FieldName]?.Reported || "",
            IsAssignToAll: false
        };
    }

    editField = () => {
        if (!this.props.Disabled && this.props.Cluster && this.props.Cluster.SupportedCommands && this.props.Cluster.SupportedCommands.indexOf("WriteAttributes") > -1)
            this.setState({
                ShowModal: true,
                Value: this.props.Cluster?.Attributes?.[this.props.FieldName]?.Reported || ""
            });
    }

    updateField = () => {
        if (this.props.Node && this.props.Node.Unid)
            if (this.state.IsAssignToAll && this.props.Node.ep)
                Object.keys(this.props.Node.ep).forEach(ep => this.update(ep));
            else this.update(this.props.EpName);
        this.setState({ ShowModal: false });
    }

    update(epName: string) {
        this.props.SocketServer.send(JSON.stringify({
            type: "update-cluster-attr",
            data: {
                Unid: `${this.props.Node.Unid}/${epName}`,
                ClusterType: this.props.ClusterName,
                Payload: { [this.props.FieldName]: this.state.Value.trim() }
            }
        }));
    }

    onChange = (event: any) => {
        this.setState({ Value: event.target.value });
    }

    onChangeAuto = (event: any, value: any) => {
        this.setState({ Value: value });
    }

    onChangeAssign = (event: any) => {
        this.setState({ IsAssignToAll: event.target.checked });
    }

    getFiled = () => {
        let options = this.props.GetOptions && this.props.GetOptions();
        return options?.length
            ? <Autocomplete
                selectOnFocus
                defaultValue={this.props.Cluster?.Attributes?.[this.props.FieldName]?.Reported || ""}
                options={options}
                onChange={this.onChangeAuto}
                freeSolo
                renderInput={(params) => <TextField {...params}
                    onChange={this.onChange}
                    size="small" className="flex-input" type="text" variant="outlined" label={this.props.FieldName}
                />}
            />
            : <TextField size="small" className="flex-input" type="text" variant="outlined" label={this.props.FieldName} value={this.state.Value} onChange={this.onChange} />
    }

    render() {
        return (
            <>
                <span className={`${this.props.Cluster && this.props.Cluster.SupportedCommands && this.props.Cluster.SupportedCommands.indexOf("WriteAttributes") > -1 ? "editable-field" : ""} ${this.props.Disabled ? "cursor-default" : ""}`} onClick={this.editField}>
                    {this.props.Cluster?.Attributes?.[this.props.FieldName]?.Reported?.trim() || (this.props.ReplaceNameWithUnid ? `${this.props.Node.Unid}/${this.props.EpName}` : "-")}
                </span>
                {this.state.ShowModal
                    ? <Modal show={true} onHide={() => this.setState({ ShowModal: false })}>
                        <Modal.Header closeButton>
                            <Modal.Title>{`Update ${this.props.FieldName}`}</Modal.Title>
                        </Modal.Header>
                        <Modal.Body>
                            {this.getFiled()}
                            {this.props.Node.ep && Object.keys(this.props.Node.ep).length
                                ? <Form.Label column sm="12" className="flex">
                                    <div className="check-container">
                                        <Form.Check defaultChecked={this.state.IsAssignToAll} onChange={this.onChangeAssign} />
                                    </div>
                                    <div className="word-break">Assign to all endpoints</div>
                                </Form.Label>
                                : null}
                        </Modal.Body>
                        <Modal.Footer>
                            <Button variant="primary" onClick={this.updateField}>Update</Button>
                            <Button variant="outline-primary" onClick={() => this.setState({ ShowModal: false })}>Cancel</Button>
                        </Modal.Footer>
                    </Modal>
                    : <></>
                }

            </>
        );
    }
}

export default EditableAttribute