export type CommandDlgState = {
    Command: any,
    Unid: any,
    ShowModal: boolean
}

export type CommandDlgProps = {
    SocketServer: WebSocket,
    ClusterType: any
}