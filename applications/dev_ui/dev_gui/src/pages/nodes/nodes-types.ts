export type NodesProps = {
  NodeList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type NodesState = {
  NodeList: any[],
  ShowSecurityModal: boolean,
  ShowRemoveModal: boolean,
  Dsk: string,
  UnidToRemove: string,
  RequestedData: any,
  Filter: string,
  ProcessingNode: string | null,
  InclusionProccess: boolean
}