export type SmartStartProps = {
  SmartStartList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type SmartStartState = {
  SmartStartList: any[],
  Filter: string,
  ShowModal: boolean
}