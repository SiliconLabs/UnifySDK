export type ConfigurationParamsProps = {
    IsConnected: boolean;
    SocketServer: WebSocket;
    NodeList: any;
    Unid: string;
    EndPoint: string;
    ParamId: string;
}

export type ConfigurationParamsState = {
    Parameter: Parameter;
}

export type Parameter = {
    ParameterId: number,
    Value: number,
    Name: string,
    Info: string,
    MinimumValue: number,
    MaximumValue: number,
    DefaultValue: number,
    DisplayFormat: string,
    ReadOnly: boolean,
    Advanced: boolean,
    AlteringCapabilities: boolean
}