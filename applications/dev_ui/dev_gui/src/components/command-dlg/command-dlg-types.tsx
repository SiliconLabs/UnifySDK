export type CommandDlgState = {
    Command: any,
    Unid: any,
    ShowModal: boolean,
    ClusterType: any | undefined
}

export type CommandDlgProps = {
    SocketServer: WebSocket,
    ClusterType?: any | undefined
}