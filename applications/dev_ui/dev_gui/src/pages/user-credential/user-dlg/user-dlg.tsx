import * as React from 'react';
import { Button, Modal } from 'react-bootstrap';
import { UserDlgProps, UserDlgState } from './user-dlg-types';
import UserCredentialAttrs from '../user-credential-attrs/user-credential-attrs';


class UserDlg extends React.Component<UserDlgProps, UserDlgState> {
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

    getAvailableUserID(userCredential: any) {
        let availableUserID = []
        var users = userCredential.User || {}
        let maxUsers = userCredential.SupportedUserUniqueIdentifiers.Reported;
        for (let i = 1; i <= maxUsers; i++) {
            if (users[i] === undefined) {
                availableUserID.push({label: i +"", id: i});
            }
        }

        return availableUserID;
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
                    if (command.name.includes("Add")) {
                        field.values = this.getAvailableUserID(userCredential);
                    } else {
                        field.values = this.getDefinedUserIDs(userCredential);
                    }
                    if (field.values.length !== 0) {
                        field.defaultValue = field.values[0];
                        field.default = field.values[0].id;
                    }
                    break;
                case "UserType":
                    field.enum = this.getSupportedEnum(field.enum, userCredential.SupportedUserTypes?.Reported);
                    break;
                case "CredentialRule":
                    field.enum = this.getSupportedEnum(field.enum, userCredential.SupportedCredentialRules?.Reported);
                    break;
                case "UserActiveState":
                    field.default = true;
                    break;
            }
            return field;
        });
        
        updatedCommand.UserList = userCredential.User;

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

export default UserDlg