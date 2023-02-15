import { TextField, Tooltip } from '@mui/material';
import React from 'react';
import { Button, Col, Row } from 'react-bootstrap';
import { Link } from 'react-router-dom';
import { ConfigurationParamsProps, ConfigurationParamsState } from './configuration-params-types';

class ConfigurationParams extends React.Component<ConfigurationParamsProps, ConfigurationParamsState> {
    constructor(props: ConfigurationParamsProps) {
        super(props);

        let node = this.props.NodeList.find((i: any) => i.Unid === this.props.Unid);
        let params = node && node.ep && node.ep[this.props.EndPoint]?.Clusters?.ConfigurationParameters?.Attributes?.ConfigurationParameters?.Reported;
        this.state = {
            Parameter: params && params.find((i: any) => i.ParameterId.toString() === this.props.ParamId)
        };
    }

    static getDerivedStateFromProps(props: ConfigurationParamsProps, state: ConfigurationParamsState) {
        let node = props.NodeList.find((i: any) => i.Unid === props.Unid);
        let params = node && node.ep && node.ep[props.EndPoint]?.Clusters?.ConfigurationParameters?.Attributes?.ConfigurationParameters?.Reported;
        return {
            Parameter: params && params.find((i: any) => i.ParameterId.toString() === props.ParamId)
        };
    }

    render() {
        return (
            <>
                <h3>The Configuration Parameter '{this.state.Parameter?.Name}' [{this.state.Parameter?.ParameterId}]</h3>
                <Row hidden={!this.props.IsConnected} >
                    <Col xs={12}>
                        <Link to={`/configurationparameters`}>
                            <Button variant="outline-primary" className="float-left margin-b-10">Back</Button>
                        </Link>
                    </Col>
                </Row>
                {
                    (!this.state.Parameter || !this.props.IsConnected)
                        ? <Row>
                            <Col xs={12} className="text-center">
                                <span className="no-content">No Content</span>
                            </Col>
                        </Row>
                        :
                        <div className="col-sm-12 cluster-attributes">
                            {Object.keys(this.state.Parameter).map((field: any, cIndex: number) => {
                                return <div key={cIndex} className={`col-xl-3 col-md-6 inline margin-v-10`}>
                                    <Tooltip title={field}>
                                        <TextField size="small" className="flex-input" fullWidth={true} inputProps={{ readOnly: true }} label={field} variant="outlined" type="text"
                                            value={(this.state.Parameter as any)[field]} />
                                    </Tooltip>
                                </div>
                            })}
                        </div>
                }
            </>
        )
    }
}

export default ConfigurationParams