export type GroupsProps = {
  NodeList: any[],
  GroupList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type GroupsState = {
  ProcessingGroup: Group,
  GroupList: any[],
  ShowEditModal: boolean,
}

export type Group = {
  GroupId: number,
  GroupName: string,
  NodeList: any,
  SupportedCommands: string[],
  FailedNodes: string[],
  UpdatingNodes: string[],
  Clusters: string[]
}