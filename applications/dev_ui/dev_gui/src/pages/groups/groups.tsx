import React from 'react';
import { Button, Card, Col, OverlayTrigger, Popover, Row, Spinner, Table } from 'react-bootstrap';
import { Group, GroupsProps, GroupsState } from './groups-types';
import * as FiIcons from 'react-icons/fi';
import * as AiIcons from 'react-icons/ai';
import ConfirmDlg from '../../components/confirm-dlg/confirm-dlg';
import GroupCommandDlg from '../../components/group-command-dlg/group-command-dlg';
import EditGroupDlg from '../../components/edit-group-dlg/edit-group-dlg';
import { Tooltip } from '@mui/material';

export class Groups extends React.Component<GroupsProps, GroupsState> {
  constructor(props: GroupsProps) {
    super(props);
    this.state = {
      ProcessingGroup: {} as Group,
      GroupList: this.getGroups(this.props.GroupList, this.props.NodeList),
      ShowEditModal: false
    };
    this.changeEditGroupDlg = React.createRef();
    this.changeConfirmDlg = React.createRef();
    this.changeGroupCommandDlg = React.createRef();
  }

  changeEditGroupDlg: any;
  changeConfirmDlg: any;
  changeGroupCommandDlg: any;

  getGroups(groupList: any[], nodeList: any[]) {
    groupList.forEach((group: any) => {
      group.Count = group.NodeList && Object.keys(group.NodeList).length;
      group.FailedNodes = ["<b>DD/fff</b>: [OnOff, DoorLock]", "<b>DD/fff</b>: [OnOff, DoorLock]"];
      group.UpdatingNodes = ["<b>DD/fff</b><br>&nbsp;&nbsp;<i>OnOff</i>:&nbsp;&nbsp;[prop1:123, prop2:563]<br>&nbsp;&nbsp;<i>DoorLock</i>:;&nbsp;&nbsp;[prop1:123, prop2:563]", "<b>DD/fff</b><br>&nbsp;&nbsp;<i>OnOff</i>:&nbsp;&nbsp;[prop1:123, prop2:563]<br>&nbsp;&nbsp;<i>DoorLock</i>:&nbsp;&nbsp;[prop1:123, prop2:563]"];
      group.Clusters = [];
      group.NodeList && Object.keys(group.NodeList).forEach((unid) => {
        let node = nodeList.find(i => i.Unid === unid);
        if (!node) return;
        group.NodeList[unid]?.forEach((ep: any) => {
          let endPoint = "ep" + ep;
          if (!node.ep || !node.ep[endPoint])
            return;
          let clusters = node.ep[endPoint].Clusters || {};
          if (node.NetworkStatus === "Offline" || node.NetworkStatus === "Unavailable") {
            group.FailedNodes.push(`<b>${node.Unid}/${endPoint}</b>:&nbsp;[${Object.keys(clusters).join(", ") || "-"}]`);
          } else {
            let updatingClusters: string[] = [];
            Object.keys(clusters).forEach((cluster: string) => {
              if (group[cluster] && group[cluster].SupportedCommands && group[cluster].SupportedCommands.length && group.Clusters.indexOf(cluster) === -1)
                group.Clusters.push(cluster);
              let attrs = clusters[cluster].Attributes || {};
              let updatingProperties: string[] = [];
              Object.keys(attrs).forEach(attr => {
                let attribute = attrs[attr];
                if (attribute.Desired !== undefined && attribute.Desired !== null) {
                  let type = typeof (attribute.Desired);
                  if (type !== "object" && (attribute.Reported !== attribute.Desired))
                    updatingProperties.push(`${attr}: ${attribute.Desired}`)
                  else if (type === "object") {
                    this.compareAttr(updatingProperties, attr, attribute.Desired, attribute.Reported);
                  }
                }
              });
              if (updatingProperties.length)
                updatingClusters.push(`&nbsp;&nbsp;<i>${cluster}</i>:&nbsp;&nbsp;[${updatingProperties.join(', ')}]`);
            });
            if (updatingClusters.length)
              group.UpdatingNodes.push(`<b>${node.Unid}/${endPoint}</b> <br>${updatingClusters.join('<br>')}`);
          }
        });
      });
    });
    return groupList.sort((a, b) => a.GroupId - b.GroupId);
  }

  compareAttr(updatingProperties: string[], parent: string, desired: any, reported: any) {
    Object.keys(desired).forEach(i => {
      if (typeof (desired[i]) === 'object')
        return this.compareAttr(updatingProperties, `${parent}.${i}`, desired[i], reported && reported[i])
      else
        if (!reported || (reported[i] !== desired[i]))
          updatingProperties.push(`${parent}.${i}: ${desired[i]}`);
    });
  }

  updateState(groupList: any[], nodeList: any[]) {
    this.setState({ GroupList: this.getGroups(groupList, nodeList) });
  }

  createGroup() {
    let maxIndex = 0;
    this.state.GroupList && this.state.GroupList.length && this.state.GroupList.forEach((item) => maxIndex = maxIndex > item.GroupId ? maxIndex : item.GroupId);
    let newGroup = {
      GroupName: "",
      GroupId: maxIndex + 1,
      NodeList: {},
      SupportedCommands: [],
      FailedNodes: [],
      UpdatingNodes: [],
      Clusters: []
    } as Group;
    this.edit(newGroup, true);
  }

  edit(group: Group, isNew: boolean) {
    this.changeEditGroupDlg.current.updateState(group, this.props.NodeList, false, isNew, this.state.GroupList.map((i: any) => i.GroupId));
  }

  remove(item: Group) {
    this.setState({ ProcessingGroup: item }, () => {
      this.changeConfirmDlg?.current.update(
        `Remove Group ${this.state.ProcessingGroup.GroupName || "-"}[${this.state.ProcessingGroup.GroupId}]`,
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
    let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    return (
      <>
        <h3>Group List</h3>
        <Row hidden={!this.props.IsConnected} >
          <Col xs={12}>
            <Button variant="outline-primary" className="float-left margin-b-10" onClick={() => this.createGroup()}>Create Group</Button>
          </Col>
        </Row>
        {(!this.state.GroupList || !this.state.GroupList.length)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : (isMobile
            ? <div className='table-content'>
              {this.state.GroupList.map((item, index) => {
                if (item.GroupId === 0) return;
                return (
                  <Card key={index} className="inline margin-v-10">
                    <Card.Header className='flex'>
                      <div className="col-sm-8">
                        <i>GroupId:</i> <b>{item.GroupId}</b>
                      </div>
                      <div className='col-sm-4'>
                        <Tooltip title="Remove" className='float-right'>
                          <span className="icon">
                            <FiIcons.FiTrash2 color="#dc3545" onClick={() => this.remove(item)} />
                          </span>
                        </Tooltip>
                        <Tooltip title="View/Edit" className='float-right'>
                          <span className="icon">
                            <FiIcons.FiEdit className="margin-h-5" onClick={() => this.edit(item, false)} />
                          </span>
                        </Tooltip>
                        <Tooltip title="Run Command" className='float-right'>
                          <span className={item.FailedNodes.length === item.Count ? `cursor-default disabled margin-h-5 icon` : `margin-h-5 icon`} >
                            <FiIcons.FiCommand onClick={() => { if (item.FailedNodes.length !== item.Count) this.choiceGroupCommand(item) }} />
                          </span>
                        </Tooltip>
                      </div>
                    </Card.Header>
                    <Card.Body>
                      <div className='col-sm-12'>
                        <div className="col-sm-6 inline"><b><i>Name: </i></b>{item.GroupName || "-"}</div>
                        <div className="col-sm-6 inline"><b><i>Apply after: </i></b>{item.ApplyAfter}</div>
                        <div className="col-sm-12 inline"><b><i>Nodes Count: </i></b>{item.Count}</div>
                        <div className="col-sm-12 inline" hidden={!item.FailedNodes.length}><b><i>Failed endpoint(s): </i></b>
                          <span hidden={!item.FailedNodes.length} className="icon margin-h-5 cursor-default"><AiIcons.AiOutlineWarning color="orange" /></span>
                          <div className="content padding-l-25" dangerouslySetInnerHTML={{ __html: item.FailedNodes.join('<br>') }}></div>
                        </div>
                        <div className="col-sm-12 inline" hidden={!item.UpdatingNodes.length}><b><i>Waiting for update: </i></b>
                          <Spinner hidden={!item.UpdatingNodes.length} as="span" animation="border" size="sm" variant="primary" />
                          <div className="content padding-l-25" dangerouslySetInnerHTML={{ __html: item.UpdatingNodes.join('<br>') }}></div>
                        </div>
                      </div>
                    </Card.Body>
                  </Card>
                );
              })}
            </div>
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
                {this.state.GroupList.map((item, index) => {
                  if (item.GroupId === 0) return;
                  let waitingPopover = (
                    <Popover id={`w${item.GroupId}`} className="popover-l">
                      <Popover.Title as="h3">Waiting for update next properties</Popover.Title>
                      <Popover.Content>
                        <div className="content padding-l-25" dangerouslySetInnerHTML={{ __html: item.UpdatingNodes.join('<br>') }}></div>
                      </Popover.Content>
                    </Popover>
                  );

                  let failedPopover = (
                    <Popover id={`f${item.GroupId}`} className="popover-l">
                      <Popover.Title as="h3">Group contains {item.FailedNodes.length} failed endpoint(s)</Popover.Title>
                      <Popover.Content>
                        <div className="content padding-l-25" dangerouslySetInnerHTML={{ __html: item.FailedNodes.join('<br>') }}></div>
                      </Popover.Content>
                    </Popover>
                  );
                  return (
                    <tr key={index}>
                      <td>{item.GroupId}</td>
                      <td>{item.GroupName || "-"}</td>
                      <td>
                        <OverlayTrigger trigger={['hover', 'focus']} placement="right" overlay={waitingPopover}>
                          <Spinner hidden={!item.UpdatingNodes.length} as="span" animation="border" size="sm" variant="primary" />
                        </OverlayTrigger>
                      </td>
                      <td>
                        <OverlayTrigger trigger={['hover', 'focus']} placement="right" overlay={failedPopover}>
                          <span hidden={!item.FailedNodes.length} className="icon margin-h-5 cursor-default"><AiIcons.AiOutlineWarning color="orange" /></span>
                        </OverlayTrigger>
                        {item.Count}
                      </td>
                      <td className="text-center">
                        <Tooltip title="Run Command">
                          <span className={item.FailedNodes.length === item.Count ? `cursor-default disabled margin-h-5 icon` : `margin-h-5 icon`} >
                            <FiIcons.FiCommand onClick={() => { if (item.FailedNodes.length !== item.Count) this.choiceGroupCommand(item) }} />
                          </span>
                        </Tooltip>
                        <Tooltip title="View/Edit">
                          <span className="icon">
                            <FiIcons.FiEdit className="margin-h-5" onClick={() => this.edit(item, false)} />
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
          )
        }

        <EditGroupDlg ref={this.changeEditGroupDlg} SocketServer={this.props.SocketServer} NodeList={this.props.NodeList} />

        <GroupCommandDlg ref={this.changeGroupCommandDlg} SocketServer={this.props.SocketServer} />

        <ConfirmDlg ConfirmAction={this.confirmRemove} ref={this.changeConfirmDlg}></ConfirmDlg>
      </>
    )
  };
}

export default Groups;