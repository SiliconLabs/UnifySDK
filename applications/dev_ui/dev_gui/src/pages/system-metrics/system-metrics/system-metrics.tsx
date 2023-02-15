import '../system-metrics.css'
import React, { Fragment } from 'react';
import { Button, Card, Col, Row } from 'react-bootstrap';
import { Link } from 'react-router-dom';
import { SystemMetricsProps, SystemMetricsState } from './system-metrics-types';
import * as Recharts from 'recharts';
import { TextField, Tooltip } from '@mui/material';


class SystemMetrics extends React.Component<SystemMetricsProps, SystemMetricsState> {
    constructor(props: SystemMetricsProps) {
        super(props);

        this.state = {
            SystemMetrics: this.props.SystemMetricsList[this.props.Mid]
        };
    }

    isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);

    static getDerivedStateFromProps(props: SystemMetricsProps, state: SystemMetricsState) {
        return {
            SystemMetrics: props.SystemMetricsList[props.Mid]
        };
    }

    getAreaChart = (dataField: string, valueName: string, title: string, yAxisProps: any, tooltipSign: string) => {
        if (this.state.SystemMetrics.Attributes[dataField] !== undefined) {
            return <Recharts.ResponsiveContainer height={235} className="col-xl-6 col-sm-12 no-padding margin-v-20">
                <Recharts.AreaChart data={this.state.SystemMetrics.Attributes[dataField].map((i: number) => { return { [valueName]: i } })} margin={{ top: 25, right: 10, left: (this.isMobile ? 105 :25), bottom: 5 }}>
                    <defs>
                        <linearGradient id="colorUv" x1="0" y1="0" x2="0" y2="1">
                            <stop offset="5%" stopColor="#83a6ed" stopOpacity={0.8} />
                            <stop offset="95%" stopColor="#83a6ed" stopOpacity={0} />
                        </linearGradient>
                    </defs>
                    <Recharts.Legend formatter={() => <span>{title}</span>} />
                    <Recharts.YAxis {...yAxisProps} />
                    <Recharts.CartesianGrid strokeDasharray="3 3" />
                    <Recharts.Tooltip labelFormatter={(i) => ""} formatter={(i: any) => `${i}%`} />
                    <Recharts.Area isAnimationActive={false} type="monotone" dataKey={valueName} stroke="#83a6ed" fillOpacity={1} fill="url(#colorUv)" />
                </Recharts.AreaChart>
            </Recharts.ResponsiveContainer>
        }
    }

    getRadialBarChart = (availableField: number, totalField: number, title: string, tooltipSign: string) => {
        if (availableField !== undefined && totalField !== undefined) {
            return <Recharts.ResponsiveContainer height={235} className="col-xl-4 col-sm-6 no-padding margin-v-20">
                <Recharts.RadialBarChart
                    height={235}
                    innerRadius="40%"
                    outerRadius="80%"
                    data={[{
                        name: title,
                        value: Math.round(availableField),
                        fill: "#83a6ed"
                    }]}
                    startAngle={230}
                    endAngle={-50}>
                    <Recharts.PolarAngleAxis type="number" domain={[0, Math.round(totalField)]} angleAxisId={0} tickCount={7} />
                    <Recharts.RadialBar angleAxisId={0} label={{ fill: '#666', position: 'insideStart' }} background dataKey='value' />
                    <Recharts.Legend iconSize={10} layout='vertical' verticalAlign='bottom' align="center" />
                </Recharts.RadialBarChart>
            </Recharts.ResponsiveContainer>
        }
    }

    getDiskCounters = (uuid: string) => {
        let counters = this.state.SystemMetrics.Attributes.DisksCounters?.filter((i: any) => i.UUID === uuid);
        if (counters && counters.length > 0) {
            return <>
                {this.getBarChart([{ firstName: "Read Count", first: counters[0].ReadCount ?? 0, secondName: "Write Count", second: counters[0].WriteCount ?? 0 }], "")}
                {this.getBarChart([{ firstName: "Read Bytes", first: counters[0].ReadBytes ?? 0, secondName: "Write Bytes", second: counters[0].WriteBytes ?? 0 }], " Bytes")}
            </>
        }
    }

    getBarChart = (data: any, tooltipSign: string) => {
        return <Recharts.ResponsiveContainer height={this.isMobile ? 300 : 235} className="col-xl-3 col-sm-6 no-padding margin-v-10">
            <Recharts.BarChart data={data}
                margin={{ top: 15, right: 30, left: (this.isMobile ? 80 : 40), bottom: 5 }}>
                <Recharts.CartesianGrid strokeDasharray="3 3" />
                <Recharts.YAxis />
                <Recharts.XAxis domain={[0, 0]} tick={false} />
                <Recharts.Legend formatter={(i) => <span>{data[0][i + "Name"]}</span>} />
                <Recharts.Bar dataKey="first" fill="#83a6ed" label={{ fill: '#666', position: 'insideStart' }} />
                <Recharts.Bar dataKey="second" fill="#82ca9d" label={{ fill: '#666', position: 'insideStart' }} />
            </Recharts.BarChart>
        </Recharts.ResponsiveContainer>
    }

    render() {
        let attrs = this.state.SystemMetrics?.Attributes;
        let cpuData = attrs?.CPUAverageUsagePercent?.map((i: number, index: number) => {
            return {
                "CPU Average Usage": i,
                "CPU Min Usage": attrs?.CPUMinUsagePercent && attrs?.CPUMinUsagePercent[index],
                "CPU Max Usage": attrs?.CPUMaxUsagePercent && attrs?.CPUMaxUsagePercent[index],
                x: null
            }
        })

        let temperature: any = undefined;
        if (attrs?.CurrentTemperatureCelcius !== undefined) {
            if (attrs.MinTemperatureCelcius !== undefined && attrs.MaxTemperatureCelcius !== undefined) {
                let range = Math.abs(attrs.MaxTemperatureCelcius - attrs.MinTemperatureCelcius);
                range = range < 10 ? 10 : range;
                temperature = {
                    Min: Math.round(attrs.MinTemperatureCelcius - range * 0.1),
                    Max: Math.round(attrs.MaxTemperatureCelcius + range * 0.1)
                };
            } else if (attrs.MinTemperatureCelcius === undefined && attrs.MaxTemperatureCelcius === undefined) {
                let range = attrs?.AverageTemperatureCelcius !== undefined ? Math.abs(attrs.CurrentTemperatureCelcius - attrs.AverageTemperatureCelcius) : 5;
                range = range < 5 ? 5 : range;
                temperature = {
                    Min: Math.round(attrs.CurrentTemperatureCelcius - range * 2),
                    Max: Math.round(attrs.CurrentTemperatureCelcius + range * 2)
                };
            } else if (attrs.MinTemperatureCelcius === undefined) {
                let range = Math.abs((attrs.AverageTemperatureCelcius ?? attrs.MaxTemperatureCelcius) - attrs.CurrentTemperatureCelcius);
                range = range < 5 ? 5 : range;
                let min = Math.round(attrs.CurrentTemperatureCelcius - range);
                range = Math.abs(attrs.MaxTemperatureCelcius - min);
                temperature = {
                    Min: min - range * 0.1,
                    Max: attrs.MaxTemperatureCelcius + range * 0.1
                };
            } else if (attrs.MaxTemperatureCelcius === undefined) {
                let range = Math.abs(attrs.CurrentTemperatureCelcius - (attrs.AverageTemperatureCelcius ?? attrs.MinTemperatureCelcius));
                range = range < 5 ? 5 : range;
                let max = Math.round(attrs.CurrentTemperatureCelcius + range);
                range = Math.abs(max - attrs.MinTemperatureCelcius);
                temperature = {
                    Min: attrs.MinTemperatureCelcius - range * 0.1,
                    Max: max + range * 0.1
                };
            }
            let step = (145 / (temperature.Max - temperature.Min));
            temperature.CurrentLength = (temperature.Max - attrs.CurrentTemperatureCelcius) * step;
            temperature.AvrgPos = (temperature.Max - attrs.AverageTemperatureCelcius) * step;
            temperature.MaxPos = (temperature.Max - attrs.MaxTemperatureCelcius) * step;
            temperature.MinPos = (temperature.Max - attrs.MinTemperatureCelcius) * step;
        }

        return (
            <>
                <h3>The System Metrics of the '{this.props.Mid}' {attrs && attrs.Hostname ? `[HostName: ${attrs.Hostname}]` : ""} {attrs && attrs.FQDN ? `[FQDN: ${attrs.FQDN}]` : ""}</h3>
                <Row hidden={!this.props.IsConnected} >
                    <Col xs={12}>
                        <Link to={`/systemmetrics`}>
                            <Button variant="outline-primary" className="float-left margin-b-10">Back</Button>
                        </Link>
                    </Col>
                </Row>
                {
                    (!attrs || !this.props.IsConnected)
                        ? <Row>
                            <Col xs={12} className="text-center">
                                <span className="no-content">No Content</span>
                            </Col>
                        </Row>
                        :
                        <div className="col-sm-12 cluster-attributes">
                            <Card hidden={attrs.CPUUsagePercent === undefined} className="padding-10">
                                <Card.Header>CPU</Card.Header>
                                <Card.Body className='inline no-padding'>
                                    {this.getAreaChart("CPUUsagePercent", "CPU Usage", "CPU Usage, %", { tickFormatter: (value: number) => `${value}%`, type: "number", domain: [0, 100] }, "%")}
                                    {this.getAreaChart("CPUFrequencyMHz", "CPU Frequency", "CPU Frequency, MHz", { tickFormatter: (value: number) => `${value}MHz`, type: "number" }, "MHz")}
                                    {!cpuData?.length
                                        ? <></>
                                        : <Recharts.ResponsiveContainer height={235} className="col-sm-12 no-padding margin-v-20">
                                            <Recharts.ComposedChart data={cpuData} margin={{ top: 25, right: 10, left: (this.isMobile ? 105 :25), bottom: 5 }}>
                                                <Recharts.Legend />
                                                <Recharts.YAxis tickFormatter={(value: number) => `${value}%`} type={"number"} domain={[0, 100]} />
                                                <Recharts.CartesianGrid strokeDasharray="3 3" />
                                                <Recharts.Tooltip labelFormatter={(i) => ""} formatter={(value: any) => `${value}%`} />
                                                <Recharts.Area isAnimationActive={false} type="monotone" dataKey="CPU Max Usage" stroke="#ca8282" fill="#ca8282" />
                                                <Recharts.Area isAnimationActive={false} type="monotone" dataKey="CPU Average Usage" stroke="#ffc658" fill="#ffc658" />
                                                <Recharts.Area isAnimationActive={false} type="monotone" dataKey="CPU Min Usage" stroke="#82ca9d" fill="#82ca9d" />
                                            </Recharts.ComposedChart >
                                        </Recharts.ResponsiveContainer>
                                    }
                                </Card.Body>
                            </Card>
                            <br />
                            <Card hidden={attrs.RAMTotalMB === undefined} className="padding-10">
                                <Card.Header>Memory</Card.Header>
                                <Card.Body className='inline no-padding'>
                                    {this.getRadialBarChart(attrs.RAMAvailableMB ?? 0, attrs.RAMTotalMB, "RAM Available, MB", "MB")}
                                    {this.getRadialBarChart(attrs.SWAPMemoryUsedMB ?? 0, attrs.SWAPMemoryTotalMB, "SWAP Memory Used, MB", "MB")}
                                    {this.getRadialBarChart(attrs.VirtualMemoryUsedMB ?? 0, attrs.VirtualMemoryTotalMB, "VirtualMemory Used, MB", "MB")}
                                </Card.Body>
                            </Card>
                            <br />
                            {attrs.DisksUsage?.map((disk: any, index: number) => {
                            return <Fragment key={index}>
                                <Card className="padding-10">
                                    <Card.Header>Disk: {disk.UUID}</Card.Header>
                                    <Card.Body className='inline no-padding'>
                                        {this.getRadialBarChart(disk.FreeSpaceMB, Math.round(disk.FreeSpaceMB / ((100 - disk.UsagePercent) / 100)), "Free Space, MB", "MB")}
                                        {this.getDiskCounters(disk.UUID)}

                                    </Card.Body>
                                </Card>
                                <br />
                            </Fragment>
                        })}
                {attrs.NetworkInterfacesData?.map((i: any, index: number) => {
                    return <Fragment key={index}>
                        <Card className="padding-10">
                            <Card.Header>Network Interface: {i.InterfaceName}</Card.Header>
                            <Card.Body className='inline no-padding'>
                                {this.getBarChart([{ firstName: "Sent Bytes", first: i.BytesSent ?? 0, secondName: "Received Bytes", second: i.BytesReceived ?? 0 }], " Bytes")}
                                {this.getBarChart([{ firstName: "Packets Sent", first: i.PacketsSent ?? 0, secondName: "Packets Received", second: i.PacketsReceived ?? 0 }], " Bytes")}
                                {this.getBarChart([{ firstName: "Errors In", first: i.ErrorsIn ?? 0, secondName: "Errors Out", second: i.ErrorsOut ?? 0 }], " Bytes")}
                                {this.getBarChart([{ firstName: "Drop In", first: i.DropIn ?? 0, secondName: "Drop Out", second: i.DropOut ?? 0 }], " Bytes")}
                            </Card.Body>
                        </Card>
                        <br />
                    </Fragment>
                })}
                <Card hidden={!attrs.WirelessNetworkInterfacesData?.length} className="padding-10">
                    <Card.Header>Wireless Network Interfaces</Card.Header>
                    <Card.Body className='inline no-padding'>
                        <Recharts.ResponsiveContainer height={235} className="col-sm-12 no-padding margin-v-20">
                            <Recharts.BarChart data={attrs.WirelessNetworkInterfacesData?.map((i: any) => { return { name: i.InterfaceName, value: i.SignalStrengthdBm } })}
                                margin={{ top: 15, right: 30, left: 20, bottom: 5 }}>
                                <Recharts.CartesianGrid strokeDasharray="3 3" />
                                <Recharts.YAxis />
                                <Recharts.XAxis dataKey="name" />
                                <Recharts.Legend formatter={() => <span>Signal Strength, dBm</span>} />
                                <Recharts.Bar dataKey="value" fill="#83a6ed" label={{ fill: '#666', position: 'insideStart' }} />
                            </Recharts.BarChart>
                        </Recharts.ResponsiveContainer>
                    </Card.Body>
                </Card>
                <br />
                <Card hidden={attrs?.BatteryPercentage === undefined && attrs?.CurrentTemperatureCelcius === undefined && attrs?.SystemInterrupts === undefined && attrs?.UptimeMinutes === undefined} className="padding-10">
                    <Card.Body>
                        <div className='col-sm-12 inline'>
                            {attrs?.BatteryPercentage === undefined
                                ? <></>
                                : <div className={`${attrs?.BatteryPercentage === 100 ? "full" : (attrs?.BatteryPercentage > 50 ? "half" : (attrs?.BatteryPercentage > 25 ? "fourth" : (!attrs.BatteryPercentage ? "empty" : "")))} col-sm-4 battery-container`}>
                                    <div className='battery-top'></div>
                                    <div className="battery">
                                        <div className='battery-inner-container'>
                                            <div className='battery-inner' style={{ height: `${Math.round(168 - 168 * ((attrs?.BatteryPercentage ?? 0) / 100))}px` }}></div>
                                            <div className='battery-line' style={{ top: "42px" }}></div>
                                            <div className='battery-line' style={{ top: "86px" }}></div>
                                            <div className='battery-line' style={{ top: "130px" }}></div>
                                            <div className='battery-value'>{attrs.BatteryPercentage ?? 0}%</div>
                                            <Tooltip title={attrs.PowerPlugged ? "Power Plugged" : "Power Unplugged"}><img className={`${attrs.PowerPlugged ? "battery-plugged" : "battery-unplugged"} battery-icon`} /></Tooltip>
                                        </div>
                                    </div>
                                </div>
                            }
                            {attrs?.CurrentTemperatureCelcius === undefined
                                ? <></>
                                : <div className='thermometr-container col-sm-4'>
                                    <div className='thermometr'>
                                        <div className='thermometr-inner'></div>
                                        <div className='thermometr-inner-value' style={{ height: `${145 - temperature.CurrentLength}px`, top: `${6 + temperature.CurrentLength}px` }}></div>
                                        <div className='thermometr-indicator-value max' style={{ top: `${temperature.MaxPos}px` }} hidden={attrs?.MaxTemperatureCelcius === undefined}>
                                            <div className='thermometr-indicator'></div>
                                            <Tooltip title="Max Temperature"><div className='thermometr-value'>{attrs?.MaxTemperatureCelcius}°C</div></Tooltip>
                                        </div>
                                        <div className='thermometr-indicator-value avrg' style={{ top: `${temperature.AvrgPos}px` }} hidden={attrs?.AverageTemperatureCelcius === undefined}>
                                            <div className='thermometr-indicator'></div>
                                            <Tooltip title="Average Temperature"><div className='thermometr-value'>{attrs?.AverageTemperatureCelcius}°C</div></Tooltip>
                                        </div>
                                        <div className='thermometr-indicator-value min' style={{ top: `${temperature.MinPos}px` }} hidden={attrs?.MinTemperatureCelcius === undefined}>
                                            <div className='thermometr-indicator'></div>
                                            <Tooltip title="Min Temperature"><div className='thermometr-value'>{attrs?.MinTemperatureCelcius}°C</div></Tooltip>
                                        </div>
                                    </div>
                                    <div className='thermometr-bottom'>
                                        <Tooltip title="Current Temperature"><div className='thermometr-bottom-inner'>
                                            <div className='current'>{attrs?.CurrentTemperatureCelcius}°C</div>
                                        </div>
                                        </Tooltip>
                                    </div>
                                </div>
                            }
                        </div>
                        <div className='col-sm-12'>
                            <div className="col-sm-6 inline margin-v-20" hidden={attrs.SystemInterrupts === undefined}>
                                <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label="System Interrupts" variant="outlined" type="text" value={attrs.SystemInterrupts || ""} />
                            </div>
                            <div className="col-sm-6 inline margin-v-20" hidden={attrs.UptimeMinutes === undefined}>
                                <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label="Uptime Minutes" variant="outlined" type="text" value={attrs.UptimeMinutes || ""} />
                            </div>
                        </div>
                    </Card.Body>
                </Card>
            </div>
                }
            </>
        )
    }
}

export default SystemMetrics