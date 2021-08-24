export type GroupsProps = {
  NodeList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type GroupsState = {
  ProcessingGroup: Group,
  GroupList: Map<number, Group>,
  ShowEditModal: boolean,
}

export type Group = {
  GroupId: number,
  Name: string,
  GroupEndpointList: any[],
  SupportedCommands: string[],
  FailedNodes: string[],
  UpdatingNodes: string[]
}