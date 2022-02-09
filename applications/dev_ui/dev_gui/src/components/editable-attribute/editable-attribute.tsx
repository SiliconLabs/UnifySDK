import * as React from 'react';
import { Button, Modal } from 'react-bootstrap';
import { TextField } from '@material-ui/core';
import { EditableAttributeProps, EditableAttributeState } from './editable-attribute-types';


class EditableAttribute extends React.Component<EditableAttributeProps, EditableAttributeState> {
    constructor(props: EditableAttributeProps) {
        super(props);
        this.state = {
            ShowModal: false,
            Value: this.props.Cluster?.Attributes?.[this.props.FieldName]?.Reported || ""
        };
    }

    editField = () => {        
        if (this.props.Cluster && this.props.Cluster.SupportedCommands && this.props.Cluster.SupportedCommands.indexOf("WriteAttributes") > -1)
            this.setState({ ShowModal: true });
    }

    updateField = () => {
        if (this.props.Unid)
            this.props.SocketServer.send(JSON.stringify({
                type: "update-cluster-attr",
                data: {
                    Unid: this.props.Unid,
                    ClusterType: this.props.ClusterName,
                    Payload: { [this.props.FieldName]: this.state.Value }
                }
            }));
        this.setState({ ShowModal: false });
    }

    onchange = (event: any) => {
        this.setState({ Value: event.target.value });
    }

    render() {
        return (
            <>
                <span className={`${this.props.Cluster && this.props.Cluster.SupportedCommands && this.props.Cluster.SupportedCommands.indexOf("WriteAttributes") > -1 ? "editable-field" : ""}`} onClick={this.editField}>{this.props.Cluster?.Attributes?.[this.props.FieldName]?.Reported || (this.props.ReplaceNameWithUnid ? this.props.Unid : "-")}</span>
                <Modal show={this.state.ShowModal} onHide={() => this.setState({ ShowModal: false })}>
                    <Modal.Header closeButton>
                        <Modal.Title>{`Update ${this.props.FieldName}`}</Modal.Title>
                    </Modal.Header>
                    <Modal.Body>
                        <TextField size="small" className="flex-input" type="text" variant="outlined" label={this.props.FieldName} value={this.state.Value} onChange={this.onchange} />
                    </Modal.Body>
                    <Modal.Footer>
                        <Button variant="primary" onClick={this.updateField}>Update</Button>
                        <Button variant="outline-primary" onClick={() => this.setState({ ShowModal: false })}>Cancel</Button>
                    </Modal.Footer>
                </Modal>
            </>
        );
    }
}

export default EditableAttribute