import { TextField, Tooltip } from '@material-ui/core';
import React from 'react';
import { Button, Col, Row } from 'react-bootstrap';
import { Link } from 'react-router-dom';
import { ClusterTypeAttrs } from '../../../cluster-types/cluster-type-attributes';
import { RFTelemetryProps, RFTelemetryState } from './rf-telemetry-types';

class RFTelemetry extends React.Component<RFTelemetryProps, RFTelemetryState> {
    constructor(props: RFTelemetryProps) {
        super(props);

        this.state = {
            Report: this.props.NodeList.find((i: any) => i.Unid === this.props.Unid && i.RFTelemetry.TxReport?.Destinations[this.props.DestinationUNID] !== undefined)
                ?.RFTelemetry.TxReport.Destinations[this.props.DestinationUNID].Report
        };
    }

    static getDerivedStateFromProps(props: RFTelemetryProps, state: RFTelemetryState) {
        return {
            Report: props.NodeList.find((i: any) => i.Unid === props.Unid && i.RFTelemetry.TxReport?.Destinations[props.DestinationUNID] !== undefined)
                ?.RFTelemetry.TxReport.Destinations[props.DestinationUNID].Report
        };
    }

    render() {
        let command = ClusterTypeAttrs.RFTelemetry.server.commands.find((i: { name: string; }) => i.name === "TxReport");
        return (
            <>
                <h3>The RF Telemetry of {this.props.Unid}</h3>
                <Row hidden={!this.props.IsConnected} >
                    <Col xs={12}>
                        <Link to={`/rftelemetry`}>
                            <Button variant="outline-primary" className="float-left margin-b-10">Back</Button>
                        </Link>
                    </Col>
                </Row>
                {
                    (!this.state.Report || !this.props.IsConnected)
                        ? <Row>
                            <Col xs={12} className="text-center">
                                <span className="no-content">No Content</span>
                            </Col>
                        </Row>
                        :
                        <div className="col-sm-12 cluster-attributes">
                            {command.fields.map((field: any, cIndex: number) => {
                                return <div key={cIndex} className={`col-sm-3 inline margin-v-10`}>
                                    <Tooltip title={field.name}>
                                        <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label={field.name} variant="outlined" type="text"
                                            value={this.state.Report[field.name]} />
                                    </Tooltip>
                                </div>

                            })}
                        </div>
                }
            </>
        )
    }
}

export default RFTelemetry