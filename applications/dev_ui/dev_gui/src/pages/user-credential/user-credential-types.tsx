export type UserCredentialProps = {
  NodeList: any,
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type UserCredentialState = {
  List: Map<string, any>,
  IsAllExpanded: boolean
}