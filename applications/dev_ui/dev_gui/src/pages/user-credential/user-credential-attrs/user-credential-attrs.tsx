import * as React from 'react';
import { Card, Form } from 'react-bootstrap';
import * as FiIcons from 'react-icons/fi';
import { Autocomplete, FormControlLabel, InputAdornment, MenuItem, Switch, TextField, Tooltip } from '@mui/material';
import CommandAttrs from '../../../components/command-atts/command-attrs';

class UserCredentialAttrs extends CommandAttrs  {
    constructor(props: {}) {
            super(props);
    }

    
    // If the command is trying to modify a user or credential, we update the payload with the current data
    // Otherwise we only update the payload UserUniqueID
    updatePayloadBasedOnUserData(userID : any, payload: any, commandName: string, userList: any, credentialType: any, credentialSlot: any) {
        if (!userList) {
            return payload;
        }

        let modifyCommand = commandName.includes("Modify");
        let credentialCommand = commandName.includes("Credential");

        if (modifyCommand) {
            let user = userList[userID];
            if (credentialCommand) {
                const userCred = user.Credential || {};
                const credType = userCred[credentialType] || {};
                const credSlot = credType[credentialSlot] || {};
                const credData = credSlot.CredentialData?.Reported || "";
                if (credData) {
                    payload.CredentialData = credData;
                } else {
                    payload.CredentialData = "";
                }
            } else {
                Object.keys(payload).forEach((key) => {
                    if (user[key]) {
                        payload[key] = user[key].Reported;
                    }
                });
            }
        }

        return payload;
    }


    updateCredentials(userUniqueId: any, credentialType: any, userList: any, currentPayload: any, command: any) {
        var slotItem  = command.fields.find((item: any) => item.name === "CredentialSlot");
        if (!slotItem)
            return currentPayload;

        if (command.name.includes("Add")) {
            slotItem.values = this.getAvailableSlots(command.UserCredential, userUniqueId, credentialType);
        } else {
            slotItem.values = this.getDefinedSlots(command.UserCredential, userUniqueId, credentialType);
        }
        const credentialSlot = slotItem.values[0]?.id || ''
        let newPayload = this.updatePayloadBasedOnUserData(currentPayload.UserUniqueID, currentPayload, command.name,
            userList, credentialType, credentialSlot);
        newPayload["CredentialSlot"] = credentialSlot;
        newPayload["CredentialType"] = credentialType;

        return newPayload;
    }
    // Called when the User ID changes in the modal
    onValueChange(valueObject: any, name: string) {
        if (!valueObject) {
            return;
        }

        let commandName = this.state.Command?.name;
        let userList = this.state.Command?.UserList;
        let currentPayload = this.state.Payload;
        const rawValue = valueObject.value?.id || valueObject.id;
        
        if (!commandName || !userList) {
            return;
        }

        let newPayload = [];
        switch(name) {
            case "UserUniqueID":
                if (commandName.includes("Credential")) {
                    newPayload = this.updateCredentials(rawValue, this.state.Payload.CredentialType, userList, currentPayload, this.state.Command);
                }

                // Update the payload based on the user ID
                newPayload = this.updatePayloadBasedOnUserData(rawValue, newPayload.length == 0 ? currentPayload : newPayload, commandName, userList, currentPayload.CredentialType, currentPayload.CredentialSlot);

                newPayload["UserUniqueID"] = rawValue;
                this.setState({ Payload: newPayload });
                break;
            case "CredentialType":
                    const userUniqueId = this.state.Payload.UserUniqueID;
                    const credentialType = rawValue;
                    newPayload = this.updateCredentials(userUniqueId, credentialType, userList, currentPayload, this.state.Command);
                    // Needed in case we don't have a CredentialSlot/UserId field
                    newPayload["CredentialType"] = credentialType;
                    this.setState({ Payload: newPayload, Command: this.state.Command });
                break;
            case "CredentialSlot":
                const credentialSlot = rawValue;
                newPayload = this.updatePayloadBasedOnUserData(currentPayload.UserUniqueID, currentPayload, commandName, userList, currentPayload.CredentialType, credentialSlot);
                newPayload["CredentialSlot"] = credentialSlot;
                this.setState({ Payload: newPayload });
                break;
        }
    }

    getAutocompleteValue(payloadValue:any) {
        return {label: payloadValue+"", id: payloadValue};
    } 
    
    getAvailableSlots(userCredentials: any, currentUserID: any, credentialType: any) {
        
        var availableSlot = [];
        var credentialTypeName = credentialType || "default";
        var currentCredentialRule = userCredentials.Credentials[credentialTypeName] || {};
        var maxSlot = currentCredentialRule.SupportedSlotCount?.Reported || 10;
        var userObject = userCredentials.User || {};
        var credentials = userObject[currentUserID]?.Credential || {};
        var currentUserSlots = credentials[credentialType] || {};

        for (let i = 1; i <= maxSlot; i++) {
            if (currentUserSlots[i] === undefined) {
                availableSlot.push({label: i +"", id: i});
            }
        }

        return availableSlot;
    }
    getDefinedSlots(userCredential: any, currentUserID: any, credentialType: any) {
        var userObject = userCredential.User || {};
        var credentials = userObject[currentUserID]?.Credential || {};
        var currentUserSlots = credentials[credentialType] || {};

        return Object.keys(currentUserSlots).map( (slot_id) => ({label: slot_id +"", id: parseInt(slot_id)}))
    }


    // This is the default payload for the command
    // We update it if we are modifying something
    getPayload(command: any) {
        let payload = super.getPayload(command);


        if (command.name.includes("Credential")) {
            payload = this.updateCredentials(payload.UserUniqueID, payload.CredentialType, command.UserList, payload, command);
        }

        return this.updatePayloadBasedOnUserData(payload.UserUniqueID, payload, command.name, command.UserList, payload.CredentialType, payload.CredentialSlot);
    }

    renderField = (item: any, payload: any, prefixNames: any[], index: any)  => {
        var validationRegex = ""
        // Expire timeout minutes is only for DuressUser
        if (item.name === "ExpiringTimeoutMinutes" && this.state.Payload?.UserType !== "DuressUser") {
            return;
        }
        if ((item.name === "CredentialData"
            && this.state.Payload?.CredentialType === "PINCode") ||
            item.name === "PINCode") {
            validationRegex = "^[0-9]*$";
        }

        switch (item.type) {
            case "number":
            if (item.values) {
                return (
                    <div key={index} className="col-sm-6 inline margin-v-10">
                        <Autocomplete
                            disablePortal
                            options={item.values}
                            defaultValue={item.defaultValue}
                            value={this.getAutocompleteValue(payload)}
                            onChange={(_,value) => this.onValueChange({value}, item.name)}
                            renderInput={(params) => <TextField {...params} className="flex-input" fullWidth={true} label={item.name} name={item.name}
                                variant="outlined" 
                            />}
                        />
                    </div>
                )
            }
            break;
            case "boolean":
                return (
                    <div key={index} className="col-sm-6 inline margin-v-10">
                        <div className="col-sm-12">
                            <div className="check-container">
                                <FormControlLabel control={<Switch checked={payload} />} label={item.name} name={item.name} onChange={this.handleChange.bind(this, prefixNames, true, false)} />
                            </div>
                        </div>
                    </div>
                )
            case "enum":
                if (item.enum && item.enum.length) {
                    return (
                        <div key={index} className="col-sm-6 inline margin-v-10">
                            <TextField size="small" className="flex-input" fullWidth={true} select label={item.name} name={item.name}
                                value={payload} onChange=
                                
                                { (event)=> {
                                    if (item.name == "CredentialType") {
                                        this.onValueChange({id: event.target.value}, item.name);
                                    } else {
                                        this.handleChange(prefixNames, false, false, event)
                                    }
                                }
                                } 
                                variant="outlined">
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
        }
        return (<div key={index} className={`col-sm-6 inline margin-v-10`}>
                        <TextField size="small" className="flex-input" fullWidth={true} label={item.name} name={item.name} variant="outlined" type={item.type}
                            value={this.getValue(item.type, payload)} 
                            onChange={ (event: any) => {
                                if (validationRegex !== "") {
                                    if (!event.target.value.match(validationRegex)) {
                                        return;
                                    }
                                }
                                if (item.maxLength && item.maxLength < event.target.value.length) {
                                    return
                                }
                                this.handleChange(prefixNames, false, item.type === "number", event)
                            }} 
                            onFocus={(event: any) => event.target.select()} inputProps={
                                { readOnly: this.state.ReadOnly }
                            } />
                </div>)
    }
}

export default UserCredentialAttrs