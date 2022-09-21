export type NetworkManagementProps = {
  NodeList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type NetworkManagementState = {
  NodeList: any[],
  ShowSecurityModal: boolean,
  ShowRemoveModal: boolean,
  Dsk: string,
  UnidToRemove: string,
  RequestedData: any,
  ProcessingNode: string | null,
  InclusionProccess: boolean,
  SortName: string,
  IsSortAcs: boolean,
}