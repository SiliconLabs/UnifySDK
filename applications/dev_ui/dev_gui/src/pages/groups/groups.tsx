import React from 'react';
import { Button, Col, OverlayTrigger, Popover, Row, Spinner, Table } from 'react-bootstrap';
import Tooltip from '@material-ui/core/Tooltip';
import { Group, GroupsProps, GroupsState } from './groups-types';
import * as FiIcons from 'react-icons/fi';
import * as AiIcons from 'react-icons/ai';
import ConfirmDlg from '../../components/confirm-dlg/confirm-dlg';
import GroupCommandDlg from '../../components/group-command-dlg/group-command-dlg';
import EditGroupDlg from '../../components/edit-group-dlg/edit-group-dlg';

export class Groups extends React.Component<GroupsProps, GroupsState> {
  constructor(props: GroupsProps) {
    super(props);
    this.state = {
      ProcessingGroup: {} as Group,
      GroupList: this.getGroups(this.props.NodeList),
      ShowEditModal: false
    };
    this.changeEditGroupDlg = React.createRef();
    this.changeConfirmDlg = React.createRef();
    this.changeGroupCommandDlg = React.createRef();
  }

  changeEditGroupDlg: any;
  changeConfirmDlg: any;
  changeGroupCommandDlg: any;

  getGroups(list: any) {
    let groups = new Map<number, any>();
    list.forEach((node: any) => {
      if (!node.ep) return;
      Object.keys(node.ep).forEach(endpoint => {
        let groupList = node.ep[endpoint].Groups?.Attributes?.GroupList?.Reported;
        if (!groupList || !groupList.length) return;
        groupList.forEach((id: any) => {
          let group = (groups.has(id)
            ? groups.get(id)
            : {
              GroupId: id,
              SupportedCommands: node.ep[endpoint].Groups.SupportedCommands,
              GroupEndpointList: [],
              FailedNodes: [],
              UpdatingNodes: []
            }) as Group;
          group.Name = group.Name || node.ep[endpoint].Groups?.Attributes[id]?.Name?.Reported;
          let clusters = node.ep[endpoint].Clusters || {};
          if (group.GroupEndpointList.filter(g => g.Unid === node.Unid && g.Ep === endpoint).length === 0)
            group.GroupEndpointList.push({
              Unid: node.Unid,
              Ep: endpoint,
              ClusterTypes: Object.keys(clusters),
              Security: node.Security,
              NetworkStatus: node.NetworkStatus
            });

          if (node.NetworkStatus === "Offline" || node.NetworkStatus === "Unavailable")
            group.FailedNodes.push(`<b>${node.Unid}/${endpoint}</b>:&nbsp;[${Object.keys(clusters).join(", ") || "-"}]`);
          else {
            let updatingClusters: string[] = [];
            Object.keys(clusters).forEach(cluster => {
              let attrs = clusters[cluster].Attributes || {};
              let updatingProperties: string[] = [];
              Object.keys(attrs).forEach(attr => {
                let attribute = attrs[attr];
                if (attribute.Desired !== undefined && attribute.Desired !== null) {
                  let type = typeof (attribute.Desired);
                  if (type !== "object" && attribute.Reported !== attribute.Desired)
                    updatingProperties.push(`${attr}: ${attribute.Desired}`)
                  else if (type === "object")
                    Object.keys(attribute.Desired).forEach(i => {
                      if (attribute.Reported[i] !== undefined && (attribute.Reported[i] !== attribute.Desired[i]))
                        updatingProperties.push(`${attr}.${i}: ${attribute.Desired[i]}`)
                    });
                }
              });
              if (updatingProperties.length)
                updatingClusters.push(`<i>${cluster}</i>:&nbsp;<br>&nbsp;&nbsp;&nbsp;&nbsp;[${updatingProperties.join(', <br>')}]`);
            });
            if (updatingClusters.length)
              group.UpdatingNodes.push(`<b>${node.Unid}/${endpoint}</b> <br>&nbsp;&nbsp;${updatingClusters.join('; <br>&nbsp')}`);
          }
          groups.set(id, group);
        });
      });
    })
    return new Map([...groups.entries()].sort((a, b) => a[0] - b[0]));
  }

  updateState(nodeList: any[]) {
    this.setState({ GroupList: this.getGroups(nodeList) });
  }

  createGroup() {
    let maxIndex = 0;
    this.state.GroupList && this.state.GroupList.size && this.state.GroupList.forEach((value, key) => maxIndex = maxIndex > key ? maxIndex : key);
    let newGroup = {
      Name: "",
      GroupId: maxIndex + 1,
      GroupEndpointList: [],
      SupportedCommands: [],
      FailedNodes: [],
      UpdatingNodes: []
    } as Group;
    this.edit(newGroup);
  }

  edit(group: Group) {
    this.changeEditGroupDlg.current.updateState(group, this.props.NodeList);
  }

  remove(item: Group) {
    this.setState({ ProcessingGroup: item }, () => {
      this.changeConfirmDlg?.current.update(
        true,
        `Remove Group ${this.state.ProcessingGroup.Name || "-"}[${this.state.ProcessingGroup.GroupId}]`,
        `Are you sure, you want to remove this group?`
      );
    });
  }

  confirmRemove = () => {
    this.sendGroupCommand(this.state.ProcessingGroup.GroupId, "Groups", "RemoveGroup", { GroupId: this.state.ProcessingGroup.GroupId });
  }

  choiceGroupCommand(item: Group) {
    this.changeGroupCommandDlg.current.updateState(item, true);
  }

  sendGroupCommand(groupId: number, clusterType: string, cmd: string, payload: any) {
    this.props.SocketServer.send(JSON.stringify(
      {
        type: "run-group-command",
        data: {
          GroupId: groupId,
          ClusterType: clusterType,
          Cmd: cmd,
          Payload: payload
        }
      }))
  }

  render() {
    return (
      <>
        <h3>Group List</h3>
        <Row hidden={!this.props.IsConnected} >
          <Col xs={12}>
            <Button variant="outline-primary" className="float-left margin-b-10" onClick={() => this.createGroup()}>Create Group</Button>
          </Col>
        </Row>
        {(!this.state.GroupList || !this.state.GroupList.size)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table striped hover>
            <thead>
              <tr className="">
                <th>GroupId</th>
                <th>Name</th>
                <th className="wd-40px"></th>
                <th>Nodes Count</th>
                <th className="wd-col-2">&ensp;</th>
              </tr>
            </thead>
            <tbody>
              {[...this.state.GroupList.values()].map((item, index) => {
                let waitingPopover = (
                  <Popover id={`w${item.GroupId}`} className="popover-l">
                    <Popover.Title as="h3">Waiting for update next properties</Popover.Title>
                    <Popover.Content>
                      <div className="content" dangerouslySetInnerHTML={{ __html: item.UpdatingNodes.join(', <br>') }}></div>
                    </Popover.Content>
                  </Popover>
                );

                let failedPopover = (
                  <Popover id={`f${item.GroupId}`} className="popover-l">
                    <Popover.Title as="h3">Group contains {item.FailedNodes.length} failed endpoint(s)</Popover.Title>
                    <Popover.Content>
                      <div className="content" dangerouslySetInnerHTML={{ __html: item.FailedNodes.join('<br>') }}></div>
                    </Popover.Content>
                  </Popover>
                );
                return (
                  <tr key={index}>
                    <td>{item.GroupId}</td>
                    <td>{item.Name || "-"}</td>
                    <td>
                      <OverlayTrigger trigger={['hover', 'focus']} placement="right" overlay={waitingPopover}>
                        <Spinner hidden={!item.UpdatingNodes.length} as="span" animation="border" size="sm" variant="primary" />
                      </OverlayTrigger>
                    </td>
                    <td>
                      <OverlayTrigger trigger={['hover', 'focus']} placement="right" overlay={failedPopover}>
                        <span hidden={!item.FailedNodes.length} className="icon margin-h-5 cursor-defult"><AiIcons.AiOutlineWarning color="orange" /></span>
                      </OverlayTrigger>
                      {item.GroupEndpointList.length}
                    </td>
                    <td className="text-center">
                      <Tooltip title="Run Command">
                        <span className={item.FailedNodes.length === item.GroupEndpointList.length ? `cursor-defult disabled margin-h-5 icon` : `margin-h-5 icon`} >
                          <FiIcons.FiCommand onClick={() => { if (item.FailedNodes.length !== item.GroupEndpointList.length) this.choiceGroupCommand(item) }} />
                        </span>
                      </Tooltip>
                      <Tooltip title="View/Edit">
                        <span className="icon">
                          <FiIcons.FiEdit className="margin-h-5" onClick={() => this.edit(item)} />
                        </span>
                      </Tooltip>
                      <Tooltip title="Remove">
                        <span className="icon">
                          <FiIcons.FiTrash2 color="#dc3545" onClick={() => this.remove(item)} />
                        </span>
                      </Tooltip>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </Table>
        }

        <EditGroupDlg ref={this.changeEditGroupDlg} SocketServer={this.props.SocketServer} NodeList={this.props.NodeList} />

        <GroupCommandDlg ref={this.changeGroupCommandDlg} SocketServer={this.props.SocketServer} />

        <ConfirmDlg ConfirmAction={this.confirmRemove} ref={this.changeConfirmDlg}></ConfirmDlg>
      </>
    )
  };
}

export default Groups;