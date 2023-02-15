export type MeasurementsProps = {
    IsConnected: boolean | null;
    SocketServer: WebSocket;
    NodeList: any;
}

export type MeasurementsState = {
    CurrentCluster: string;
}