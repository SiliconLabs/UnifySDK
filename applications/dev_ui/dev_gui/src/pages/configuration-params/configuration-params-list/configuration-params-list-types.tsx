export type ConfigurationParamsListProps = {
  NodeList: any,
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type ConfigurationParamsListState = {
  List: Map<string, any>,
  IsAllExpanded: boolean
}