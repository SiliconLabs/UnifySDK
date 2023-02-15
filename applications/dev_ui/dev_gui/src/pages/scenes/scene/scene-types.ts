export type SceneProps = {
  GroupList: any[],
  NodeList: any[],
  IsConnected: boolean | null,
  SocketServer: WebSocket,
  GroupID: string,
  SceneID: string,
  Unid: string,
  Ep: string
}

export type SceneState = {
  Scene: SceneItem,
  Cluster: string,
  GetScene: any,
  ClusterToAdd: string,
  IsEdit: boolean,
  AvailableClusters: string[]
}

export type SceneItem = {
  GroupID: number | null,
  SceneID: number | null,
  SceneName: string,
  TransitionTime: number,
  TransitionTime100ms: number,
  SceneTableExtensions: any
}