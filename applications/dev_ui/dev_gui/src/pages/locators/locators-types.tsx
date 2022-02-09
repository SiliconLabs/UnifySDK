export type LocatorsProps = {
    IsConnected: boolean | null;
    SocketServer: WebSocket;
    NodeList: any;
}

export type LocatorsState = {
    ClusterName: string;
    ReportName: string;
    ShowReport: boolean;
}