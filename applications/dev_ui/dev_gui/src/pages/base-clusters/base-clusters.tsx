import React from 'react';
import { Button, Col, OverlayTrigger, Popover, Row, Spinner, Table } from 'react-bootstrap';
import * as AiIcons from 'react-icons/ai';
import * as RiIcons from 'react-icons/ri';
import * as MdIcons from 'react-icons/md'
import { BaseClustersProps, BaseClustersState, ColorPickerProps } from './base-cluster-types';
import CommandDlg from '../../components/command-dlg/command-dlg';
import ClusterAttr from '../../components/cluster-attributes/cluster-attributes';
import EditGroupDlg from '../../components/edit-group-dlg/edit-group-dlg';
import { Group } from '../groups/groups-types';
import EditableAttribute from '../../components/editable-attribute/editable-attribute';
import { ChromePicker } from 'react-color';
import { Tooltip } from '@mui/material';
import { ClusterTypes } from '../../cluster-types/cluster-types';


export class BaseClusters extends React.Component<BaseClustersProps, BaseClustersState> {
  constructor(props: BaseClustersProps) {
    super(props);
    this.state = {
      ClusterList: this.getClusterList(this.props.NodeList) || [],
      ColorPicker: {
        ShowColorPicker: false,
        ColorPickerValue: { hex: '#fff' },
        ColorPickerPosition: { top: 250, left: 250 },
        ProcessedItem: {}
      } as ColorPickerProps
    };

    this.changeEditGroupDlg = React.createRef();
    this.changeCommandDlg = React.createRef();
    this.changeClusterAttr = React.createRef();
  }

  changeEditGroupDlg: any;
  changeCommandDlg: any;
  changeClusterAttr: any;

  hslToHex = (h: number, s: number, l: number) => {
    l /= 100;
    const a = s * Math.min(l, 1 - l) / 100;
    const f = (n: number) => {
      const k = (n + h / 30) % 12;
      const color = l - a * Math.max(Math.min(k - 3, 9 - k, 1), -1);
      return Math.round(255 * color).toString(16).padStart(2, '0');
    };
    return `#${f(0)}${f(8)}${f(4)}`;
  }

  getClusterList(nodeList: any[]) {
    if (!nodeList || !nodeList.length)
      return;
    let clusters: any[] = [];
    nodeList.forEach(i => {
      if (i.ClusterTypes.indexOf(this.props.ClusterType) > -1) {
        return Object.keys(i.ep).forEach((item: any) => {
          if (i.ep[item].Clusters?.[this.props.ClusterType]) {
            let cluster = { ...i.ep[item].Clusters[this.props.ClusterType] };
            cluster.Unid = i.Unid;
            cluster.NetworkStatus = i.NetworkStatus;
            cluster.Ep = item;
            cluster.NameAndLocation = i.ep[item].Clusters.NameAndLocation;
            clusters.push(cluster);
            if (this.props.ClusterType === ClusterTypes.ColorControl)
              cluster.CurrentLevel = i.ep[item].Clusters[ClusterTypes.Level]?.Attributes?.CurrentLevel?.Reported
          }
        })
      }
    });
    return clusters;
  }

  updateState(nodeList: any[]) {
    this.setState({ ClusterList: this.getClusterList(nodeList) || [] });
  }

  preSendCommand(cluster: any, cmd: string) {
    let command = this.props.ClusterTypeAttrs.server.commands.find((i: { name: string; }) => i.name === cmd);
    if (!command)
      return;

    if (command && command.fields && command.fields.length) {
      this.changeCommandDlg.current.updateState(`${cluster.Unid}/${cluster.Ep}`, command, true);
    } else
      this.props.SocketServer.send(JSON.stringify(
        {
          type: "run-cluster-command",
          data: {
            Unid: `${cluster.Unid}/${cluster.Ep}`,
            ClusterType: this.props.ClusterType,
            Cmd: command.name,
            Payload: {}
          }
        }));
  }

  showClusterAttr(item: any) {
    if (item.NetworkStatus !== "Offline" && item.NetworkStatus !== "Unavailable" && item.Attributes)
      this.changeClusterAttr.current.updateState(`${item.Unid}/${item.Ep}`, item.Attributes, item.SupportedCommands)
  }

  getNodesByClusterType(nodeList: any[]) {
    return nodeList.filter(i => i.ClusterTypes.indexOf(this.props.ClusterType) > -1);
  }

  createGroup() {
    let maxIndex = 0;
    this.props.GroupList.forEach((group) => {
      maxIndex = maxIndex > group.GroupId ? maxIndex : group.GroupId;
    });

    let newGroup = {
      GroupName: "",
      GroupId: maxIndex + 1,
      NodeList: {},
      SupportedCommands: [],
      FailedNodes: [],
      UpdatingNodes: [],
      Clusters: []
    } as Group;
    this.changeEditGroupDlg.current.updateState(newGroup, this.getNodesByClusterType(this.props.NodeList), true, true);
  }

  getUpdatingProperties(item: any) {
    let updatingProperties: string[] = [];
    if (item.Attributes)
      Object.keys(item.Attributes).forEach(attr => {
        let attribute = item.Attributes[attr];
        if (attribute.Desired !== undefined && attribute.Desired !== null) {
          let type = typeof (attribute.Desired);
          if (type !== "object" && attribute.Reported !== attribute.Desired)
            updatingProperties.push(`${attr}: ${attribute.Desired}`)
          else if (type === "object")
            Object.keys(attribute.Desired).forEach(i => {
              if (attribute.Reported !== undefined && attribute.Reported[i] !== undefined && (attribute.Reported[i] !== attribute.Desired[i]))
                updatingProperties.push(`${attr}.${i}: ${attribute.Desired[i]}`)
            });
        }
      });
    return updatingProperties;
  }

  handleColorPickerClose = () => {
    let picker = this.state.ColorPicker;
    picker.ShowColorPicker = false
    this.setState({ ColorPicker: picker });
  }

  handleColorChangeComplete = (color: any) => {
    if (!this.state.ColorPicker?.ProcessedItem) return;
    if (color.hsl.l !== this.state.ColorPicker.ColorPickerValue.hsl?.l) {
      this.props.SocketServer.send(JSON.stringify(
        {
          type: "run-cluster-command",
          data: {
            Unid: `${this.state.ColorPicker.ProcessedItem.Unid}/${this.state.ColorPicker.ProcessedItem.Ep}`,
            ClusterType: "Level",
            Cmd: "MoveToLevel",
            Payload: {
              Level: Math.round(color.hsl.l * 100),
              TransitionTime: 0,
              OptionsMask: {
                ExecuteIfOff: false,
                CoupleColorTempToLevel: false
              },
              OptionsOverride: {
                ExecuteIfOff: false,
                CoupleColorTempToLevel: false
              }
            }
          }
        }));
    }
    if (color.hsl.h !== this.state.ColorPicker.ColorPickerValue.hsl?.h || color.hsl.s !== this.state.ColorPicker.ColorPickerValue.hsl?.s) {
      this.props.SocketServer.send(JSON.stringify(
        {
          type: "run-cluster-command",
          data: {
            Unid: `${this.state.ColorPicker.ProcessedItem.Unid}/${this.state.ColorPicker.ProcessedItem.Ep}`,
            ClusterType: this.props.ClusterType,
            Cmd: "MoveToHueAndSaturation",
            Payload: {
              Hue: Math.round(color.hsl.h / 360 * 254),
              Saturation: Math.round(color.hsl.s * 254),
              TransitionTime: 0,
              OptionsMask: { ExecuteIfOff: false },
              OptionsOverride: { ExecuteIfOff: false }
            }
          }
        }));
    }
    let picker = this.state.ColorPicker;
    picker.ColorPickerValue = color;
    this.setState({ ColorPicker: picker });
  };

  getColorPicker = (item: any) => {
    let hue = item.Attributes?.CurrentHue?.Reported === undefined ? 211 : item.Attributes.CurrentHue.Reported / 254 * 360;
    let saturation = item.Attributes?.CurrentSaturation?.Reported === undefined ? 100 : item.Attributes.CurrentSaturation.Reported / 254 * 100;
    let level = item.CurrentLevel ?? 50;
    return <Tooltip title="Set Hue, Saturation and Level">
      <span className="pointer">
        <MdIcons.MdInvertColors color={`hsl(${hue}deg ${saturation}% ${level > 90 ? 90 : level}%)`} onClick={(event: any) => {
          let picker = this.state.ColorPicker;
          picker.ShowColorPicker = !this.state.ColorPicker.ShowColorPicker;
          picker.ColorPickerPosition = { top: event.clientY, left: event.clientX };
          picker.ProcessedItem = item;
          picker.ColorPickerValue.hex = this.hslToHex(hue, saturation, level);
          this.setState({ ColorPicker: picker })
        }} />
      </span>
    </Tooltip>
  }

  render() {
    let attrNames: string[] = this.props.ClusterViewOverrides?.ViewTable?.map(i => i.Name) || [];
    if (!attrNames.length && this.props.ClusterTypeAttrs.server.attributes && this.state.ClusterList && this.state.ClusterList.length) {
      let attrs = new Set<string>();
      this.state.ClusterList?.forEach((i: any) => {
        if (i.Attributes) {
          Object.keys(i.Attributes).forEach(attr => {
            attrs.add(attr);
          });
          return;
        }
      });
      for (let index = 0; index < this.props.ClusterTypeAttrs.server.attributes.length; index++) {
        if (attrs.has(this.props.ClusterTypeAttrs.server.attributes[index].name)) {
          attrNames.push(this.props.ClusterTypeAttrs.server.attributes[index].name)
          if (attrNames.length >= 5)
            break;
        }

      }
    }

    return (
      <>
        <h3 hidden={!this.props.ShowTitle}>{this.props.ClusterViewOverrides?.NavbarItem?.title || this.props.ClusterType}</h3>
        <Row hidden={!this.props.IsConnected} >
          <Col xs={12}>
            <Button variant="outline-primary" className="float-left margin-b-10" onClick={() => this.createGroup()}>Create Group</Button>
          </Col>
        </Row>
        {(!this.state.ClusterList || !this.state.ClusterList.length)
          ? <Row>
            <Col xs={12} className="text-center">
              <span className="no-content">No Content</span>
            </Col>
          </Row>
          : <Table striped hover className='vertical-middle'>
            <thead>
              <tr>
                <th>Name</th>
                {attrNames.map((item, index) => {
                  return <th key={index}>{item}</th>
                })}
                <th className="wd-40px">&nbsp;</th>
                <th>Supported Commands</th>
                <th></th>
              </tr>
            </thead>
            <tbody>
              {this.state.ClusterList.map((item, index) => {
                let updatingProperties: string[] = this.getUpdatingProperties(item);
                let popover = (
                  <Popover id={`${item.Unid}/${item.Ep}`} className="popover-l">
                    <Popover.Title as="h3">Waiting for update next properties</Popover.Title>
                    <Popover.Content>
                      {updatingProperties.join(', \n')}
                    </Popover.Content>
                  </Popover>
                );
                return <Tooltip key={index} title={(item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable") ? "Node is Offline" : ""}>
                  <tr className={(item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable") ? "disabled" : ""}>
                    <td>
                      <span hidden={item.NetworkStatus !== "Offline" || item.NetworkStatus === "Unavailable"} className="margin-h-5"><RiIcons.RiWifiOffLine color="red" /></span>
                      <EditableAttribute Node={item} EpName={item.Ep} Cluster={item.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name" SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} />
                    </td>
                    {this.props.ClusterViewOverrides?.ViewTable?.map((column, colIndex) => {
                      return <td key={colIndex}>{column.Value(item, this.getColorPicker)}</td>
                    })
                      || attrNames.map((column, colIndex) => {
                        return <td key={colIndex}>{item.Attributes && item.Attributes[column] && item.Attributes[column].Reported !== null && item.Attributes[column].Reported !== undefined ? JSON.stringify(item.Attributes[column].Reported) : "-"}</td>
                      })
                    }
                    <td>
                      <OverlayTrigger trigger={['hover', 'focus']} placement="right" overlay={popover}>
                        <Spinner hidden={!updatingProperties.length || item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} as="span" animation="border" size="sm" variant="primary" />
                      </OverlayTrigger>
                    </td>
                    <td>{item.SupportedCommands && item.SupportedCommands.filter((cmd: any) => cmd !== "WriteAttributes").map((cmd: string, index: number) => {
                      return <Button disabled={item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable"} key={index} className="margin-5" size="sm" onClick={this.preSendCommand.bind(this, item, cmd)}>{cmd}</Button>
                    })}
                    </td>
                    <td className="text-center">
                      <Tooltip title="Customize Attributes">
                        <span className={(item.NetworkStatus === "Offline" || item.NetworkStatus === "Unavailable" || !item.Attributes) ? "margin-h-5 icon cursor-default disabled" : "margin-h-5 icon"}>
                          <AiIcons.AiOutlineTool onClick={() => this.showClusterAttr(item)} />
                        </span>
                      </Tooltip>
                    </td>
                  </tr>
                </Tooltip>
              })}
            </tbody>
          </Table>
        }

        <div className="color-picker-container" hidden={!this.state.ColorPicker.ShowColorPicker} style={{
          top: this.state.ColorPicker.ColorPickerPosition.top - 96,
          left: this.state.ColorPicker.ColorPickerPosition.left - 34
        }}>
          <div className='cover' onClick={this.handleColorPickerClose} />
          <ChromePicker disableAlpha={true} color={this.state.ColorPicker.ColorPickerValue.hex} onChangeComplete={this.handleColorChangeComplete} />
        </div>

        <EditGroupDlg ref={this.changeEditGroupDlg}
          SocketServer={this.props.SocketServer}
          NodeList={this.getNodesByClusterType(this.props.NodeList)} />

        <CommandDlg ref={this.changeCommandDlg}
          SocketServer={this.props.SocketServer}
          ClusterType={this.props.ClusterType} />

        <ClusterAttr ref={this.changeClusterAttr}
          SocketServer={this.props.SocketServer}
          ClusterType={this.props.ClusterType}
          ClusterAttr={this.props.ClusterTypeAttrs.server.attributes} />
      </>
    )
  };
}

export default BaseClusters;