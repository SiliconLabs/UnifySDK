import { Tooltip } from '@material-ui/core';
import React from 'react';
import { Button, Col, Row, Table } from 'react-bootstrap';
import { Link } from 'react-router-dom';
import { Frequencies, Modes, Speed, TraceItem, UPTITraceProps, UPTITraceState } from './upti-trace-types';
import './upti-trace.css'

class UPTITrace extends React.Component<UPTITraceProps, UPTITraceState> {
    constructor(props: UPTITraceProps) {
        super(props);

        this.state = {
            SelectedPTI: this.props.UPTI.List.find(i => i.SerialNumber === this.props.SerialNumber)
        };
    }

    toHex(array: number[]) {
        return array && Array.from(array, (num) => {
            return this.toByte(num);
        }).join(' ');
    }

    toByte(num: number) {
        return ('0' + num.toString(16).toUpperCase()).slice(-2);
    }

    static getDerivedStateFromProps(props: UPTITraceProps, state: UPTITraceState) {
        return {
            SelectedPTI: props.UPTI.List.find(i => i.SerialNumber === props.SerialNumber)
        };
    }

    onPtiChange = (event: any) => {
        this.setState({ SelectedPTI: event.target.value });
    }

    getTime(timeStr: string) {
        let time = new Date(timeStr);
        return `${String(time.getHours()).padStart(2, '0')}:${String(time.getMinutes()).padStart(2, '0')}:${String(time.getSeconds()).padStart(2, '0')}.${String(time.getMilliseconds()).padStart(3, '0')}`;
    }

    render() {
        return (
            <>
                <h3>The Trace of {this.state.SelectedPTI?.SerialNumber}</h3>
                <Row hidden={!this.props.IsConnected} >
                    <Col xs={12}>
                        <Link to={`/upti`}>
                            <Button variant="outline-primary" className="float-left margin-b-10">Back</Button>
                        </Link>
                    </Col>
                </Row>
                {
                    (!this.state.SelectedPTI || !this.props.UPTI.Trace[this.state.SelectedPTI.SerialNumber] || !this.props.UPTI.Trace[this.state.SelectedPTI.SerialNumber].length || !this.props.IsConnected)
                        ? <Row>
                            <Col xs={12} className="text-center">
                                <span className="no-content">No Content</span>
                            </Col>
                        </Row>
                        : <Table striped hover>
                            <thead>
                                <tr>
                                    <th>Time</th>
                                    <th>Mode</th>
                                    <th>Protocol</th>
                                    <th>
                                        <Tooltip title="Radio Config">
                                            <span>Radio</span>
                                        </Tooltip>
                                    </th>
                                    <th>RSSI</th>
                                    <th>
                                        <Tooltip title="Channel">
                                            <span>CH</span>
                                        </Tooltip>
                                    </th>
                                    <th>Speed</th>
                                    <th>Syncword</th>
                                    <th>Data</th>
                                </tr>
                            </thead>
                            <tbody>
                                {this.props.UPTI.Trace[this.state.SelectedPTI.SerialNumber].map((item: TraceItem, index: number) => {
                                    let data = item.AdditionalData;
                                    return (
                                        <tr key={index} className={data && (data.Mode === Modes[Modes.RxAbort] || data.Mode === Modes[Modes.TxAbort] || data.Mode === Modes[Modes.None]) ? "error-background" : ""} >
                                            <td className="time">{this.getTime(item.TimeStamp)}</td>
                                            <td className="no-brake-word text-center">{data ? data.Mode : "-"}</td>
                                            <td className="text-center">{data ? (data.Protocol === 6 ? "Z-Wave" : (data.Protocol === 3 ? "BLE" : "802.15.4")) : "-"}</td>
                                            <td className="text-center">{data ? (data.Protocol === 6 ? Frequencies[data.RadioConfig & 0x1F] : "0x" + this.toByte(data.RadioConfig)) : "-"}</td>
                                            <td className="text-center">{!data || data.Mode === Modes[Modes.TxAbort] || data.Mode === Modes[Modes.TxSuccess] ? "-" : data.Rssi}</td>
                                            <td className="text-center">{data ? data.Channel : "-"}</td>
                                            <td className="text-center">{data ? Speed[data.Speed] : "-"}</td>
                                            <td className="text-center">{(data && data.Syncword.length && this.toHex(data.Syncword)) || "-"}</td>
                                            <td><b>RawFrame: </b>{this.toHex(item.RawFrame)}
                                                <br hidden={!data} />
                                                <b hidden={!data}>Data: </b>{data && this.toHex(data.Payload)}
                                            </td>
                                        </tr>
                                    );
                                })}
                            </tbody>
                        </Table>
                }
            </>
        )
    }
}

export default UPTITrace