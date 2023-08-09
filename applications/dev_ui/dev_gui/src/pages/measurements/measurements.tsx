import React from 'react';
import { Card, Col, Row, Tab, Table, Tabs } from 'react-bootstrap';
import { MeasurementsProps, MeasurementsState } from './measurements-types';
import * as RiIcons from 'react-icons/ri';
import EditableAttribute from '../../components/editable-attribute/editable-attribute';

class Measurements extends React.Component<MeasurementsProps, MeasurementsState> {
    constructor(props: MeasurementsProps) {
        super(props);
        this.state = {
            CurrentCluster: "All"
        };
    }

    getAllTable = (list: any) => {
        let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
        return (!list.length)
            ? <Row>
                <Col xs={12} className="text-center">
                    <span className="no-content">No Content</span>
                </Col>
            </Row>
            : (isMobile
                ? <div className='table-content'>
                    {list.map((node: any, index: number) => {
                        let isOffline = node.NetworkStatus === "Offline" || node.NetworkStatus === "Unavailable";
                        let className = isOffline ? "disabled" : "";
                        return Object.keys(node.ep).map((ep: any, indexEp: number) => {
                            let clusters = Object.keys(node.ep[ep].Measurements);
                            return <Card key={indexEp} className="inline margin-v-10">
                                <Card.Header className='flex'>
                                    <div className={`col-sm-6 ${className}`}>
                                        <span hidden={!isOffline} className="margin-h-5"><RiIcons.RiWifiOffLine color="red" /></span>
                                        <b>
                                            <EditableAttribute Node={node} EpName={ep} Cluster={node.ep[ep].Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
                                                SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={isOffline} />
                                        </b>
                                    </div>
                                </Card.Header>
                                <Card.Body className={className}>
                                    <div className='col-sm-12'>
                                        {clusters.map((cluster: any, indexCl: number) => {
                                            let attrs = Object.keys(node.ep[ep].Measurements[cluster].Attributes);
                                            return <Card key={indexCl} className="inline margin-v-10 small">
                                                <Card.Header className='flex'>
                                                    <div className="col-sm-6"><b>{cluster}</b></div>
                                                </Card.Header>
                                                <Card.Body>
                                                    <div className='col-sm-12'>
                                                        {attrs?.length && attrs.map((attr: string, indexAttr) => {
                                                            return <div className="col-sm-6 inline" key={indexAttr}><b><i>{attr}: </i></b>{JSON.stringify(node.ep[ep].Measurements[cluster].Attributes[attr].Reported)}</div>
                                                        })}
                                                    </div>
                                                </Card.Body>
                                            </Card>
                                        })}
                                    </div>
                                </Card.Body>
                            </Card>
                        });
                    })}
                </div>
                : <Table hover>
                    <thead>
                        <tr>
                            <th>Name</th>
                            <th>Cluster</th>
                            <th>Attributes</th>
                        </tr>
                    </thead>
                    <tbody>
                        {list.map((node: any, index: number) => {
                            let isOffline = node.NetworkStatus === "Offline" || node.NetworkStatus === "Unavailable";
                            let className = isOffline ? "disabled" : "";
                            return Object.keys(node.ep).map((ep: any, indexEp: number) => {
                                let clusters = Object.keys(node.ep[ep].Measurements);
                                return clusters.map((cluster: any, indexCl: number) => {
                                    return (
                                        <tr key={`${index}-${indexEp}-${indexCl}`}>
                                            <td className={`vertical-middle ${className}`} rowSpan={clusters.length} hidden={indexCl > 0}><EditableAttribute Node={node} EpName={ep} Cluster={node.ep[ep].Clusters.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name" SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={node.NetworkStatus === "Offline" || node.NetworkStatus === "Unavailable"} /></td>
                                            <td className={`vertical-middle ${className}`}>{cluster}</td>
                                            <td className={`vertical-middle ${className}`}>
                                                {node.ep[ep].Measurements[cluster].Attributes && Object.keys(node.ep[ep].Measurements[cluster].Attributes).map((attr: any, indexAttr: number) => {
                                                    return <div key={`${index}-${indexEp}-${indexCl}-${indexAttr}`}><b><i>{attr}: </i></b>{JSON.stringify(node.ep[ep].Measurements[cluster].Attributes[attr].Reported)}</div>
                                                })}
                                            </td>
                                        </tr>
                                    );
                                });
                            });
                        })}
                    </tbody>
                </Table>
            )
    }

    getClusterTable = (list: any) => {
        let isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
        return (!list.length)
            ? <Row>
                <Col xs={12} className="text-center">
                    <span className="no-content">No Content</span>
                </Col>
            </Row>
            : (isMobile
                ? <div className='table-content'>
                    {list.map((item: any, index: number) => {
                        let isOffline = item.Node.NetworkStatus === "Offline" || item.Node.NetworkStatus === "Unavailable";
                        let className = isOffline ? "disabled" : "";
                        return <Card key={index} className="inline margin-v-10">
                            <Card.Header className='flex'>
                                <div className={`col-sm-6 ${className}`}>
                                    <span hidden={!isOffline} className="margin-h-5"><RiIcons.RiWifiOffLine color="red" /></span>
                                    <b>
                                        <EditableAttribute Node={item.Node} EpName={item.Ep} Cluster={item.Node.ep[item.Ep].Clusters?.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name"
                                            SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={isOffline} />
                                    </b>
                                </div>
                            </Card.Header>
                            <Card.Body className={className}>
                                <div className='col-sm-12'>
                                    {item.Attributes && Object.keys(item.Attributes).map((attr: any, indexCl: number) => {
                                        return <div className="col-sm-6 inline" key={indexCl}><b><i>{attr}: </i></b>{JSON.stringify(item.Attributes[attr].Reported)}</div>
                                    })}
                                </div>
                            </Card.Body>
                        </Card>
                    })}
                </div>
                : <Table hover>
                    <thead>
                        <tr>
                            <th>Name</th>
                            <th>Attributes</th>
                        </tr>
                    </thead>
                    <tbody>
                        {list.map((item: any, index: number) => {
                            let isOffline = item.Node.NetworkStatus === "Offline" || item.Node.NetworkStatus === "Unavailable";
                            let className = isOffline ? "disabled" : "";
                            return (
                                <tr key={index}>
                                    <td className={`vertical-middle ${className}`}><EditableAttribute Node={item.Node} EpName={item.Ep} Cluster={item.Node.ep[item.Ep].Clusters.NameAndLocation} ClusterName="NameAndLocation" FieldName="Name" SocketServer={this.props.SocketServer} ReplaceNameWithUnid={true} Disabled={item.Node.NetworkStatus === "Offline" || item.Node.NetworkStatus === "Unavailable"} /></td>
                                    <td className={`vertical-middle ${className}`}>
                                        {item.Attributes && Object.keys(item.Attributes).map((attr: any, indexAttr: number) => {
                                            return <div key={indexAttr}><b><i>{attr}: </i></b>{JSON.stringify(item.Attributes[attr].Reported)}</div>
                                        })}
                                    </td>
                                </tr>
                            );
                        })}
                    </tbody>
                </Table>
            )
    }

    render() {
        let clusters: any[] = [];
        let list: any[] = [];
        this.props.NodeList.forEach((node: any) => {
            node.ep && Object.keys(node.ep).forEach((ep: any) => {
                node.ep[ep].Measurements && Object.keys(node.ep[ep].Measurements).forEach((cluster: any) => {
                    if (clusters.indexOf(cluster) === -1)
                        clusters.push(cluster);
                    if (this.state.CurrentCluster === "All" && list.filter(i => i.Unid === node.Unid)?.length === 0) {
                        list.push(node);
                    } else if (cluster == this.state.CurrentCluster) {
                        list.push({ Node: node, Ep: ep, Attributes: node.ep[ep].Measurements[cluster].Attributes });
                    }
                });
            });
        });
        return (
            <>
                <h3>Measurements</h3>
                <Tabs hidden={!this.props.IsConnected} defaultActiveKey={this.state.CurrentCluster} onSelect={(k: string | null) => this.setState({ CurrentCluster: k || "All" })}>
                    <Tab eventKey="All" title="All Measurements">
                        {this.state.CurrentCluster === "All" ? this.getAllTable(list) : null}
                    </Tab>
                    {clusters.map((cluster: any, index: number) => {
                        return <Tab eventKey={cluster} title={cluster} key={index}>
                            {this.state.CurrentCluster === cluster ? this.getClusterTable(list) : null}
                        </Tab>
                    })}
                </Tabs>
            </>
        )
    }
}

export default Measurements