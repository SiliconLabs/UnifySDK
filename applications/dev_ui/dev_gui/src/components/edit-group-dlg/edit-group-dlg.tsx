import { Tooltip } from '@material-ui/core';
import * as React from 'react';
import { Button, Col, Form, Modal, Row, Table } from 'react-bootstrap';
import { NodeTypesList } from '../../cluster-types/cluster-types';
import { Group } from '../../pages/groups/groups-types';
import { EditGroupDlgProps, EditGroupDlgState } from './edit-group-dlg-types';
import * as RiIcons from 'react-icons/ri';
import ClusterTypeTooltip from '../cluster-type-tooltip/cluster-type-tooltip';


class EditGroupDlg extends React.Component<EditGroupDlgProps, EditGroupDlgState> {
    constructor(props: EditGroupDlgProps) {
        super(props);
        this.state = {
            ProcessingGroup: {} as Group,
            PristineGroupName: "",
            IsGroupValid: true,
            EndpointList: [],
            ClusterType: null,
            CheckedAll: false,
            ShowModal: false
        };
    }
    updates: any;

    getEndpointList(nodeList: any) {
        let endpointList: any[] = [];
        nodeList && nodeList.forEach((node: any) => {
            if (node.Type !== NodeTypesList.ProtocolController && node.ep) {
                Object.keys(node.ep).forEach(endpoint => {
                    if (node.ep[endpoint].Groups?.SupportedCommands?.indexOf("AddGroup") > -1
                        && ((node.NetworkStatus !== "Offline" && node.NetworkStatus !== "Unavailable") || this.isGroupContainsEp(node.Unid, endpoint)))
                        endpointList.push({
                            Unid: node.Unid,
                            Ep: endpoint,
                            EpData: { [endpoint]: node.ep[endpoint] },
                            Security: node.Security,
                            NetworkStatus: node.NetworkStatus,
                            Groups: node.ep[endpoint].Groups,
                            Checked: this.state.CheckedAll || this.isGroupContainsEp(node.Unid, endpoint)
                        });
                });
            }
        });
        return endpointList.sort((a, b) => {
            if (a.Unid > b.Unid)
                return 1;
            if (a.Unid < b.Unid)
                return -1;
            return 0;
        });
    }

    isGroupContainsEp(unid: string, ep: string) {
        let isContains = false;
        let match = ep.match(/\d+/);
        let endPoint = match ? parseInt(match[0]) : null;
        if (endPoint !== null) {
            this.state.ProcessingGroup.NodeList && Object.keys(this.state.ProcessingGroup.NodeList).forEach(nodeId => {
                if (nodeId === unid && this.state.ProcessingGroup.NodeList[nodeId].indexOf(endPoint) > -1) {
                    isContains = true;
                    return;
                }
            });
        }
        return isContains;
    }

    updateState(item: Group, nodeList: any[], checkedAll: boolean = false) {
        this.setState({ ProcessingGroup: item, PristineGroupName: item.GroupName, CheckedAll: checkedAll }, () => {
            this.setState({ EndpointList: this.getEndpointList(nodeList) }, () => {
                this.updates = {
                    addList: checkedAll ? new Set<string>(this.state.EndpointList.map(item => `${item.Unid}/${item.Ep}`)) : new Set<string>(),
                    removeList: new Set<string>()
                };
                this.toggleEditModal(true);
            });
        });
    }

    toggleEditModal = (value: boolean) => {
        this.setState({ ShowModal: value });
    }

    handleNameChange = (event: any) => {
        let group = { ...this.state.ProcessingGroup };
        group.GroupName = event.target.value;
        this.setState({ ProcessingGroup: group });
    }

    handleCheckboxChange(item: any, event?: any) {
        let unid = `${item.Unid}/${item.Ep}`;
        item.Checked = event.target.checked;
        if (event.target.checked) {
            if (!this.isGroupContainsEp(item.Unid, item.Ep))
                this.updates.addList.add(unid);
            if (this.updates.removeList.has(unid))
                this.updates.removeList.delete(unid);
        }
        else {
            if (this.isGroupContainsEp(item.Unid, item.Ep))
                this.updates.removeList.add(unid);
            if (this.updates.addList.has(unid))
                this.updates.addList.delete(unid);
        }
        let allChecked = true;
        this.state.EndpointList.forEach(i => {
            allChecked = allChecked && i.Checked
        });
        this.setState({ EndpointList: this.state.EndpointList, CheckedAll: allChecked }, () => this.isGroupValid());
    }

    handleCheckAllChange(event: any) {
        this.updates = {
            addList: new Set<string>(),
            removeList: new Set<string>()
        };

        this.state.EndpointList.forEach(item => {
            item.Checked = event.target.checked;
            if (event.target.checked) {
                this.updates.addList.add(`${item.Unid}/${item.Ep}`)
            }
            else {
                this.updates.removeList.add(`${item.Unid}/${item.Ep}`)
            }
        });
        this.setState({ EndpointList: this.state.EndpointList, CheckedAll: event.target.checked }, () => this.isGroupValid());
    }

    updateGroup() {
        if (!this.isGroupValid())
            return;

        (this.updates.addList as Set<string>).forEach((i) => {
            this.sendClusterCommand(i, "AddGroup", { GroupId: this.state.ProcessingGroup.GroupId, GroupName: this.state.ProcessingGroup.GroupName });
        });
        (this.updates.removeList as Set<string>).forEach((i) => {
            this.sendClusterCommand(i, "RemoveGroup", { GroupId: this.state.ProcessingGroup.GroupId });
        });

        if (this.state.ProcessingGroup.GroupName !== this.state.PristineGroupName)
            this.props.SocketServer.send(JSON.stringify(
                {
                    type: "run-group-command",
                    data: {
                        GroupId: this.state.ProcessingGroup.GroupId,
                        ClusterType: "Groups",
                        Cmd: "AddGroup",
                        Payload: { GroupId: this.state.ProcessingGroup.GroupId, GroupName: this.state.ProcessingGroup.GroupName || "" }
                    }
                }));

        this.toggleEditModal(false);
    }

    sendClusterCommand(unid: string, cmd: string, payload: any) {
        this.props.SocketServer.send(JSON.stringify(
            {
                type: "run-cluster-group-command",
                data: {
                    Unid: unid,
                    Cmd: cmd,
                    Payload: payload
                }
            }));
    }

    isGroupValid() {
        let valid = !!this.state.EndpointList.find(i => i.Checked);
        this.setState({ IsGroupValid: valid });
        return valid;
    }

    render() {
        return (
            <Modal show={this.state.ShowModal} size="lg" onHide={() => this.toggleEditModal(false)}>
                <Modal.Header closeButton>
                    <Modal.Title>
                        {`Group: [${this.state.ProcessingGroup.GroupId}] ${this.state.ProcessingGroup.GroupName}`}
                    </Modal.Title>
                </Modal.Header>
                <div className="error-modal-header col-sm-12" hidden={this.state.IsGroupValid}>The group must have at least one endpoint.</div>
                <Modal.Body>
                    <div className="row col-sm-12 margin-b-10 flex">
                        <Form.Label column sm="2">Group Name:</Form.Label>
                        <Form.Control type="text" className="col-sm-3" placeholder="Group Name" defaultValue={this.state.ProcessingGroup.GroupName} onKeyUp={this.handleNameChange} />
                    </div>
                    {(!this.state.EndpointList || !this.state.EndpointList.length)
                        ? <Row>
                            <Col xs={12} className="text-center">
                                <span className="no-content">No Content</span>
                            </Col>
                        </Row>
                        : <Table striped hover>
                            <thead>
                                <tr>
                                    <th className="wd-col-3">Unid</th>
                                    <th className="wd-col-2">Type</th>
                                    <th>Security</th>
                                    <th className="text-center"><Form.Check name="CheckedAll" checked={this.state.CheckedAll} onChange={(e) => this.handleCheckAllChange(e)} /></th>
                                </tr>
                            </thead>
                            <tbody>
                                {this.state.EndpointList.map((item, index) => {
                                    let title = item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable" ? "Endpoint is Offline" : "";
                                    return (
                                        <Tooltip key={index} title={title}>
                                            <tr key={index} className={(item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable") ? "disabled" : ""}>
                                                <td>
                                                    <span hidden={!(item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable")} className="margin-h-5"><RiIcons.RiWifiOffLine color="red" /></span>
                                                    {item.Unid}/{item.Ep}
                                                </td>
                                                <td className="flex"><ClusterTypeTooltip Ep={item.EpData} /></td>
                                                <td>{item.Security}</td>
                                                <td className="text-center">
                                                    <Form.Check name={item.Unid} checked={item.Checked} onChange={(e) => this.handleCheckboxChange(item, e)} />
                                                </td>
                                            </tr>
                                        </Tooltip>
                                    );
                                })}
                            </tbody>
                        </Table>
                    }

                </Modal.Body>
                <Modal.Footer>
                    <Button variant="outline-primary" disabled={!this.state.IsGroupValid} onClick={() => this.updateGroup()}>Save</Button>
                    <Button variant="primary" onClick={() => this.toggleEditModal(false)}>Cancel</Button>
                </Modal.Footer>
            </Modal>
        );
    }
}

export default EditGroupDlg