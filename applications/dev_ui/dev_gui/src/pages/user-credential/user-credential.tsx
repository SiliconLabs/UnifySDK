import React, { Fragment } from 'react';
import * as FiIcons from 'react-icons/fi';
import * as BsIcons from 'react-icons/bs';
import * as RiIcons from 'react-icons/ri';
import { Button, Col, Dropdown, DropdownButton, Row, Table, Badge, Modal} from 'react-bootstrap';
import * as CgIcons from 'react-icons/cg';
import { Link } from 'react-router-dom';
import { UserCredentialProps, UserCredentialState } from './user-credential-types';
import EditableAttribute from '../../components/editable-attribute/editable-attribute';
import UserDlg from './user-dlg/user-dlg';
import CredDlg from './cred-dlg/cred-dlg';
import { ClusterTypeAttrs } from '../../cluster-types/cluster-type-attributes';
import { Tooltip } from '@mui/material';


export class UserCredential extends React.Component<UserCredentialProps, UserCredentialState> {
  constructor(props: UserCredentialProps) {
    super(props);
    this.state = {
      List: this.getList(this.props.NodeList),
      IsAllExpanded: false
    }
    this.userCommandDlg = React.createRef();
    this.credCommandDlg = React.createRef();
  }

  userCommandDlg: any;
  credCommandDlg: any;
  UserCredentialType = "UserCredential";

  // https://stackoverflow.com/questions/45167565/does-react-js-support-html5-datalist
  // Datalist ?
  // https://stackoverflow.com/questions/57401738/how-to-make-react-input-a-text-and-select
  // Use custom modal and preSendCommand to send MQTT command 

  preSendCommand(unid: string, endPoint: string, cmd: string) {
    let command = ClusterTypeAttrs[this.UserCredentialType].server.commands.find((i: { name: string; }) => i.name === cmd);
    if (!command)
      return;

    if (command && command.fields && command.fields.length) {
      let userCredential = this.state.List.get(unid + endPoint)?.UserCredential
      // Order matter here, a credential command should display the credCommandDlg even if it contains "user"
      if (command.name.includes("Credential") || command.name.includes("PIN")) {
        this.credCommandDlg.current.updateState(`${unid}/${endPoint}`, command, true, userCredential);
      } else if (command.name.includes("User")) {
        this.userCommandDlg.current.updateState(`${unid}/${endPoint}`, command, true, userCredential);
      } 
    } else
      this.props.SocketServer.send(JSON.stringify(
        {
          type: "run-cluster-command",
          data: {
            Unid: `${unid}/${endPoint}`,
            ClusterType: this.UserCredentialType,
            Cmd: command.name,
            Payload: {}
          }
        }));
  }

  updateState(nodeList: any) {
    this.setState({ List: this.getList(nodeList) });
  }

  getList(nodeList: any) {
    let list = new Map<string, any>();
    nodeList.forEach((item: any) => {
      if (!item.ep)
        return;
      Object.keys(item.ep).forEach(ep => {
        if (!item.ep[ep].Clusters?.UserCredential)
          return;
        let userCredential = item.ep[ep].Clusters?.UserCredential?.Attributes;
        console.log("UserCredential : ", userCredential);
        list.set(item.Unid + ep, {
          Node: item,
          EndPoint: ep,
          UserCredential: userCredential,
          IsExpanded: (this.state && this.state.List && this.state.List.get(item.Unid + ep)?.IsExpanded) ?? userCredential?.length < 3
        });
      });
    });
    return list;
  }

  getUsersHeaders = () => {
    return (<tr>
      <th>UID</th>
      <th>User Name</th>
      <th>User Type</th>
      <th>Credential Rule</th>
      <th>User Active</th>
    </tr>)
  }
  getUsersContents = (user_id: number, user: any) => {
    return (
      <>
        <td rowSpan={2}>
          <Tooltip title={<span>{["Modifier Type : " + user.UserModifierType?.Reported ,<br />,"Modifier Node ID : " + user.UserModifierNodeId?.Reported]}</span>}>
            <span>{user_id}</span>
          </Tooltip>
        </td>
        <td>
          <Tooltip title={"Encoding : " + user.UserNameEncoding?.Reported}>
            {user.UserName?.Reported}
          </Tooltip>
        </td>
        <td>
          <Tooltip title={user.ExpiringTimeoutMinutes?.Reported ? "Expiring timeout (minutes) :" + user.ExpiringTimeoutMinutes.Reported : ""}>
            <Badge pill variant="secondary">{user.UserType?.Reported}</Badge>
          </Tooltip>
        </td>
        <td><Badge pill variant="secondary">{user.CredentialRule?.Reported}</Badge></td>
        <td>{user.UserActiveState?.Reported == 1 ? <Badge pill variant="success">Active</Badge> : <Badge pill variant="danger">Inactive</Badge>}</td>
      </>
    );
  }

  getBadges = (value: any) => {
     return (value != null ? <div>
                  {Object.entries(value).map( ([key, value]) => {
                    return value ? <Badge pill variant="primary" key={key}>{key}</Badge> : '';
                  })}
                 </div>
             : "-");
  }
  getSupportedCredentialType(credentialsTypes: any, credentialRules: any) {
    return (credentialsTypes != null ? <div>
      {Object.entries(credentialsTypes).map(([credentialsType, supported]) => {
        var currentCredentialRule = credentialRules[credentialsType] || {};
        var learnSupport = currentCredentialRule.LearnSupport?.Reported;
        var supportedSlotCount = currentCredentialRule.SupportedSlotCount?.Reported;
        var credMinLength = currentCredentialRule.CredentialMinLength?.Reported;
        var credMaxLength = currentCredentialRule.CredentialMaxLength?.Reported;

        return supported ? (
        <Badge pill variant="primary" key={credentialsType}>
          <Tooltip title={["Learn Support : " + learnSupport, <br/>, "Supported Slot Count : " + supportedSlotCount, <br/>, "Credential Min Length : " + credMinLength, <br/>, "Credential Max Length : " + credMaxLength]}>
            <span>{credentialsType}</span>
          </Tooltip>
        </Badge>)
          : '';
      })}
    </div>
      : "-");
  }

  getCredentialsTable = (credential : any) => {
    if (Object.keys(credential).length === 0) {
      return (<td colSpan={8} className="cluster-info text-center"><i><b>Not credential found</b></i></td>)
    }
    return (
      <td colSpan={8} style={{backgroundColor: 'rgba(0,0,0,0.05)'}}>
        <Table size="sm">
          <thead>
            <tr>
              <th>Credential Type</th>
              <th>Credential Slot</th>
              <th className='text-center'><Tooltip title="Credential Read Back"><span>CRB</span></Tooltip></th>
              <th>Credential Data</th>
            </tr>
          </thead>
          <tbody>
            {
              Object.entries(credential).map(([credentialType, slots], index) => {
                var slotObjects = slots as object;
                var slotCount = Object.keys(slotObjects).length;
                return (
                    <>
                    {
                      Object.entries(slotObjects).map(([slotID, slotData], index) => {
                        return (<>
                          <tr key={index}>
                          { index === 0 ? <td rowSpan={slotCount}><Badge pill variant="secondary">{credentialType}</Badge></td> : <></>}
                          <td>
                            <Tooltip key={index} title={<span>{["Modifier Type : " + slotData.CredentialModifierType?.Reported, <br />, "Modifier Node ID : " + slotData.CredentialModifierNodeId?.Reported]}</span>}>
                              <span>{slotID}</span>
                            </Tooltip>
                          </td>
                          <td className="text-center">{slotData.CredentialReadBack?.Reported ? <RiIcons.RiCheckboxCircleFill color="green"></RiIcons.RiCheckboxCircleFill> : <RiIcons.RiCloseCircleFill color="red"></RiIcons.RiCloseCircleFill>} </td>
                          <td>{slotData.CredentialData?.Reported}</td>
                          </tr>
                        </>
                        )
                      })
                    }
                    </>
                )
              })}
          </tbody>
        </Table>
      </td>
    )
  }

  toggleIsAllExpanded = () => {
    this.state.List.forEach((value: any, key: string) => {
      value.IsExpanded = !this.state.IsAllExpanded;
    });
    this.setState({ IsAllExpanded: !this.state.IsAllExpanded });
  }

  render() {
    return (
      <>
        <h3>User Credential</h3>
        {(this.state.List.size === 0 || !this.props.IsConnected)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table>
              <thead>
                <tr>
                  <th className="no-padding-h">
                    <span className='icon small' onClick={this.toggleIsAllExpanded}>
                      {this.state.IsAllExpanded ? <BsIcons.BsDashSquare /> : <BsIcons.BsPlusSquare />}
                    </span>
                  </th>
                  <th>Node</th>
                  <th>Max Users</th>
                  <th>Supported User Types</th>
                  <th>Supported Credential Types</th>
                  <th>Supported Credential Rules</th>
                  <th>Admin PIN Code</th>
                  <th>Supported Commands</th>
                </tr>
              </thead>
              <tbody>
                {[...this.state.List.values()].map((item: any, index: number) => {
                  
                  let isOffline = item.Node.NetworkStatus === "Offline" || item.Node.NetworkStatus === "Unavailable";
                  let commands = item.Node.ep[item.EndPoint].Clusters.UserCredential.SupportedCommands && item.Node.ep[item.EndPoint].Clusters.UserCredential.SupportedCommands.filter((cmd: any) => cmd !== "WriteAttributes");
                  let userList = item.UserCredential?.User || {};
                  let userCount = Object.keys(userList).length;
                  return (
                    <Fragment key={index}>
                      <Tooltip key={index} title={isOffline ? "Node is Offline" : ""}>
                        <tr className={isOffline ? "disabled" : ""}>
                          <td rowSpan={2} className="no-padding-h">
                            <span className='icon small' onClick={() => { item.IsExpanded = !item.IsExpanded; }} hidden={!userCount}>
                              {item.IsExpanded ? <BsIcons.BsDashSquare /> : <BsIcons.BsPlusSquare />}</span>
                          </td>
                          <td rowSpan={2}>
                            <EditableAttribute Node={item.Node} EpName={item.EndPoint} Cluster={item.Node.ep[item.EndPoint].Clusters.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
                              SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={isOffline} />
                          </td>
            
                          <td>{item.UserCredential?.SupportedUserUniqueIdentifiers?.Reported}</td>
                          <td>{this.getBadges(item.UserCredential?.SupportedUserTypes?.Reported)}</td>
                          <td>{this.getSupportedCredentialType(item.UserCredential?.SupportedCredentialTypes?.Reported, item.UserCredential?.Credentials)}</td>
                          <td>{this.getBadges(item.UserCredential?.SupportedCredentialRules?.Reported)}</td>
                          <td>{(item.UserCredential?.SupportAdminPinCode?.Reported) ?
                            <Tooltip key={index} title={"Deactivation supported : " + item.UserCredential?.SupportAdminPinCodeDeactivation?.Reported}>
                              <span>{item.UserCredential?.AdminPinCode?.Reported}</span>
                            </Tooltip>
                            : <RiIcons.RiCloseCircleFill color="red"/>
                          }
                          </td>
                          <td className="float-left">
                            {commands && commands.length
                              ? <DropdownButton menuAlign={'right'} variant="outline-primary" title="Commands" className="float-right" disabled={isOffline} size="sm">
                                {commands.map((cmd: string, cmdIndex: number) => {
                                  return (
                                    <Dropdown.Item key={cmdIndex} onClick={this.preSendCommand.bind(this, item.Node.Unid, item.EndPoint, cmd)}> {cmd}</Dropdown.Item>
                                  )
                                })
                                }
                              </DropdownButton>
                              : <></>}
                          </td>
                        </tr>
                      </Tooltip>
                      <tr className={isOffline ? "disabled" : ""}>
                        {userCount && item.IsExpanded
                          ? 
                          (
                            <td colSpan={8}>
                            <Table>
                              <thead>
                                {this.getUsersHeaders()}
                              </thead>
                              <tbody>
                                  {Object.entries(userList).map((user: any, indexParam: number) => {
                                    var userId = user[0];
                                    var userObject = user[1];

                                    if (!userObject) {
                                      return;
                                    }

                                    var credentials = userObject.Credential || {};

                                    return (
                                    <>
                                    <tr key={indexParam} className={isOffline ? "disabled" : ""}>
                                      {this.getUsersContents(userId, userObject)}
                                    </tr>
                                    {this.getCredentialsTable(credentials)}
                                    </>
                                    )
                                  })
                                  }
                                  
                              </tbody>
                            </Table>
                            </td>
                          )
                          : <td colSpan={8} className="cluster-info text-center"><i><b>User Count:</b> {userCount}</i></td>
                        }
                      </tr>
                    </Fragment>
                  )
                })}
              </tbody>
            </Table>
        }
        
        <UserDlg ref={this.userCommandDlg}
          SocketServer={this.props.SocketServer}
        />

        <CredDlg ref={this.credCommandDlg}
          SocketServer={this.props.SocketServer}
        />

      </>
    )
  };
}

export default UserCredential;