export type RFTelemetryProps = {
    IsConnected: boolean;
    SocketServer: WebSocket;
    NodeList: any;
    Unid: string;
    DestinationUNID: string;
}

export type RFTelemetryState = {
    Report: any;
}