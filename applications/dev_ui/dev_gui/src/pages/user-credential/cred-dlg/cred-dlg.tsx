import * as React from 'react';
import { Button, Modal } from 'react-bootstrap';
import { UserDlgProps, UserDlgState } from './cred-dlg-types';
import UserCredentialAttrs from '../user-credential-attrs/user-credential-attrs';


class CredDlg extends React.Component<UserDlgProps, UserDlgState> {
    constructor(props: UserDlgProps) {
        super(props);
        this.sendCommand = this.sendCommand.bind(this);
        this.toggleModal = this.toggleModal.bind(this);
        this.state = {
            Command: {},
            Unid: "",
            ShowModal: false,
            UserCredential: {}
        };
        this.changeCommandAttrs = React.createRef();
    }
    changeCommandAttrs: any;

    toggleModal(value: boolean) {
        this.setState({ ShowModal: value });
    }

    getDefinedUserIDs(userCredential: any) {
        var users = userCredential.User
        if (!users) {
            return [];
        }
        return Object.keys(users).map(user_id => {
            return { label: user_id, id: parseInt(user_id) }
        });
    }

    getSupportedEnum(enumData: any, supportedEnumField: any) {
        if (!supportedEnumField) {
            return enumData;
        }
        return enumData.filter((enumItem:any) => supportedEnumField[enumItem.name]);
    }

    updateState(unid: string, command: any, showModal: boolean, userCredential: any) {
        let updatedCommand = structuredClone(command);

        updatedCommand.fields = command.fields.map( (field: any) => {
            switch (field.name) {
                case "UserUniqueID":
                    field.values = this.getDefinedUserIDs(userCredential);
                    if (field.values.length !== 0) {
                        field.defaultValue = field.values[0];
                        field.default = field.values[0].id;
                    }
                    break;
                case "CredentialType":
                    field.enum = this.getSupportedEnum(field.enum, userCredential.SupportedCredentialTypes?.Reported);
                    break;
            }
            return field;
        });
        

        updatedCommand.UserList = userCredential.User;
        updatedCommand.UserCredential = userCredential;
        
        this.setState({ Unid: unid, Command: updatedCommand, ShowModal: showModal, UserCredential: userCredential }, 
            () => {
                this.changeCommandAttrs.current.updateState(this.state.Command)
            });

    }

    sendCommand() {
        if (this.state.UserCredential !== undefined)
            this.props.SocketServer.send(JSON.stringify(
                {
                    type: "run-cluster-command",
                    data: {
                        Unid: this.state.Unid,
                        ClusterType: "UserCredential",
                        Cmd: this.state.Command.name,
                        Payload: this.changeCommandAttrs.current.state.Payload
                    }
                }));
    }


    render() {
        return (
            <Modal show={this.state.ShowModal} size="lg" onHide={() => this.toggleModal(false)}>
                <Modal.Header closeButton>
                    <Modal.Title>{this.state.Command.name}</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                    <UserCredentialAttrs ref={this.changeCommandAttrs} />
                </Modal.Body>
                <Modal.Footer>
                    <Button variant="primary" onClick={() => { this.sendCommand(); this.toggleModal(false); }}>
                        Send
                    </Button>
                    <Button variant="outline-primary" onClick={() => this.toggleModal(false)}>
                        Cancel
                    </Button>
                </Modal.Footer>
            </Modal>
        );
    }
}

export default CredDlg