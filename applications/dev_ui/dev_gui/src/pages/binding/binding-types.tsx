export type BindingProps = {
  NodeList: any,
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type BindingState = {
  List: Map<string, any>,
  IsAllExpanded: boolean,
  ShowBindModel: boolean,
  ShowUnBindModel: boolean,
  UnBindCluster: any,
  BindCmd: any
}