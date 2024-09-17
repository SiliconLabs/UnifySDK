export type UserDlgState = {
    Command: any,
    Unid: any,
    ShowModal: boolean,
    UserCredential: any
}

export type UserDlgProps = {
    SocketServer: WebSocket
}