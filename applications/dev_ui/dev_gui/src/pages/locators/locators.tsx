import { Tooltip } from '@material-ui/core';
import React from 'react';
import { Button, Col, Modal, Row, Tab, Table, Tabs } from 'react-bootstrap';
import { LocatorsProps, LocatorsState } from './locators-types';
import * as AiIcons from 'react-icons/ai';
import * as CgIcons from 'react-icons/cg';
import * as FiIcons from 'react-icons/fi';
import ClusterAttr from '../../components/cluster-attributes/cluster-attributes';
import { ClusterTypes } from '../../cluster-types/cluster-types';
import { ClusterTypeAttrs } from '../../cluster-types/cluster-type-attributes';
import CommandAttrs from '../../components/command-atts/command-attrs';
import EditableAttribute from '../../components/editable-attribute/editable-attribute';

class Locators extends React.Component<LocatorsProps, LocatorsState> {
    constructor(props: LocatorsProps) {
        super(props);
        this.state = {
            ClusterName: "AoXLocator",
            ReportName: "AoXLocator",
            ShowReport: false
        };
        this.changeClusterAttr = React.createRef();
        this.changeCommandAttrs = React.createRef();
    }

    changeClusterAttr: any;
    changeCommandAttrs: any;

    showClusterAttr(node: any, ep: string, tag: any) {
        if (node.NetworkStatus !== "Offline" && node.NetworkStatus !== "Unavailable" && tag.Attributes)
            this.changeClusterAttr.current.updateState(`${node.Unid}/${ep}`, tag.Attributes, tag.SupportedCommands)
    }

    getPositionTable = (list: any) => {
        return (!list.length || this.state.ClusterName !== "AoXPositionEstimation")
            ? <Row>
                <Col xs={12} className="text-center">
                    <span className="no-content">No Content</span>
                </Col>
            </Row>
            : <Table hover>
                <thead>
                    <tr>
                        <th>Name</th>
                        <th>AoX Positioning Application ID</th>
                        <th className='wd-120px'>Position</th>
                        <th className='wd-120px'>Deviation</th>
                        <th>Sequence</th>
                    </tr>
                </thead>
                <tbody>
                    {list.map((node: any, index: number) => {
                        return Object.keys(node.ep).map((ep: any, indexEp: number) => {
                            let cluster = node.ep[ep].Clusters.AoXPositionEstimation;
                            if (!cluster) return null;
                            return (
                                <tr key={`${index}-${indexEp}`}>
                                    <td className="vertical-middle"><EditableAttribute Unid={`${node.Unid}/${ep}`} Cluster={node.ep[ep].Clusters.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name" SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} /></td>
                                    <td className="vertical-middle">{cluster.Attributes?.Position?.Reported?.ApplicationId}</td>
                                    <td className="vertical-middle">{cluster.Attributes?.Position?.Reported
                                        ? `X:${cluster.Attributes.Position.Reported.X?.toFixed(4)}, Y:${cluster.Attributes.Position.Reported.Y?.toFixed(4)}, Z:${cluster.Attributes.Position.Reported.Z?.toFixed(4)}`
                                        : "-"}</td>
                                    <td className="vertical-middle">{cluster.Attributes?.Position?.Reported
                                        ? `X:${cluster.Attributes.Position.Reported.DeviationX?.toFixed(4)}, Y:${cluster.Attributes.Position.Reported.DeviationY?.toFixed(4)}, Z:${cluster.Attributes.Position.Reported.DeviationZ?.toFixed(4)}`
                                        : "-"}</td>
                                    <td className="vertical-middle">{cluster.Attributes?.Position?.Reported.Sequence}</td>
                                </tr>
                            );
                        });
                    })}
                </tbody>
            </Table>
    }

    getLocatorTable = (list: any) => {
        return (!list.length || this.state.ClusterName !== "AoXLocator")
            ? <Row>
                <Col xs={12} className="text-center">
                    <span className="no-content">No Content</span>
                </Col>
            </Row>
            : <Table>
                <thead>
                    <tr>
                        <th>Name</th>
                        <th>Reporting Mode</th>
                        <th className="text-center">Position/Orientation Valid</th>
                        <th>Position/Orientation</th>
                        <th>AzimuthMask</th>
                        <th>ElevationMask</th>
                        <th>AllowList</th>
                    </tr>
                </thead>
                <tbody>
                    {list.map((node: any, index: number) => {
                        return Object.keys(node.ep).map((ep: any, indexEp: number) => {
                            let cluster = node.ep[ep].Clusters.AoXLocator;
                            if (!cluster) return null;
                            let reportName = cluster.Attributes?.ReportingMode?.Reported;
                            if (!reportName && cluster.GeneratedCommands)
                                if (cluster.GeneratedCommands.AngleReport !== undefined)
                                    reportName = "AngleReport";
                                else if (cluster.GeneratedCommands.IQReport !== undefined)
                                    reportName = "IQReport";
                            let reportCommand = reportName && cluster.GeneratedCommands && cluster.GeneratedCommands[reportName];
                            return (
                                <tr key={`${index}-${indexEp}`}>
                                    <td className="vertical-middle"><EditableAttribute Unid={`${node.Unid}/${ep}`} Cluster={node.ep[ep].Clusters.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name" SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} /></td>
                                    <td className="vertical-middle">{cluster?.Attributes?.ReportingMode?.Reported || "-"}</td>
                                    <td className="text-center">{cluster?.Attributes?.PositionAndOrientationValid?.Reported === true ? <FiIcons.FiCheck color="#28a745" /> : (cluster?.Attributes?.AoXLocatorConfigured?.Reported === false ? <FiIcons.FiXCircle color="#6c757d" /> : "-")}</td>
                                    <td>{this.getObjectFieldValue(cluster?.Attributes?.PositionAndOrientation?.Reported)}</td>
                                    <td>{this.getObjectFieldValue(cluster?.Attributes?.AzimuthMask?.Reported)}</td>
                                    <td>{this.getObjectFieldValue(cluster?.Attributes?.ElevationMask?.Reported)}</td>
                                    <td>{this.getObjectFieldValue(cluster?.Attributes?.AllowList?.Reported)}</td>
                                    <td>
                                        <Tooltip title="Customize Attributes" className="float-right">
                                            <span className={(node.NetworkStatus === "Offline" || node.NetworkStatus === "Unavailable" || !cluster?.Attributes) ? "margin-h-5 icon cursor-defult disabled" : "margin-h-5 icon"}>
                                                <AiIcons.AiOutlineTool onClick={() => this.showClusterAttr(node, ep, cluster)} />
                                            </span>
                                        </Tooltip>
                                        <Tooltip title="View Report" className="float-right">
                                            <span className="icon" hidden={!reportCommand}>
                                                <CgIcons.CgEye className="margin-h-5" onClick={() => this.viewReport(reportName, reportCommand)} />
                                            </span>
                                        </Tooltip>
                                    </td>
                                </tr>
                            );
                        });
                    })}
                </tbody>
            </Table >
    }

    getObjectFieldValue(value: any) {
        return value !== undefined ? JSON.stringify(value).replaceAll(/,"/g, "; ").replaceAll(/},{/g, "}, {").replaceAll(/"/g, "") : "-"
    }

    toggleReport = (value: boolean) => {
        this.setState({ ShowReport: value });
    }

    viewReport = (reportName: string, payload: any) => {
        if (!reportName)
            return;
        let command = ClusterTypeAttrs.AoXLocator?.server.commands.find((i: { name: string; }) => i.name === reportName);
        if (!command || !command.fields || command.fields.length === 0)
            return;
        this.setState({ ReportName: reportName, ShowReport: true }, () => this.changeCommandAttrs.current.updateState(command, payload, true));
    }

    render() {
        let list = this.props.NodeList.filter((i: any) => i.ClusterTypes.indexOf(this.state.ClusterName) > -1);
        return (
            <>
                <Tabs hidden={!this.props.IsConnected} defaultActiveKey={this.state.ClusterName} onSelect={(k: string | null) => this.setState({ ClusterName: k || "" })}>
                    <Tab eventKey="AoXLocator" title="Locator">
                        {this.getLocatorTable(list)}
                    </Tab>
                    <Tab eventKey="AoXPositionEstimation" title="Position Estimation">
                        {this.getPositionTable(list)}
                    </Tab>
                </Tabs>
                <ClusterAttr ref={this.changeClusterAttr}
                    SocketServer={this.props.SocketServer}
                    ClusterType={this.state.ClusterName as ClusterTypes}
                    ClusterAttr={ClusterTypeAttrs.AoXLocator.server.attributes} />

                <Modal show={this.state.ShowReport} size="lg" onHide={() => this.toggleReport(false)}>
                    <Modal.Header closeButton>
                        <Modal.Title>{this.state.ReportName}</Modal.Title>
                    </Modal.Header>
                    <Modal.Body>
                        <CommandAttrs ref={this.changeCommandAttrs} />
                    </Modal.Body>
                    <Modal.Footer>
                        <Button variant="outline-primary" onClick={() => this.toggleReport(false)}>
                            Ok
                        </Button>
                    </Modal.Footer>
                </Modal>
            </>
        )
    }
}

export default Locators