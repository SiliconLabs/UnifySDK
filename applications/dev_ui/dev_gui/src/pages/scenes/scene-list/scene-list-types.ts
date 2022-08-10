export type SceneListProps = {
  SceneList: any,
  GroupList: any[],
  SocketServer: WebSocket,
  NodeList: any[],
  IsConnected: boolean | null,
  Storage: any
}

export type SceneListState = {
  ProcessingScene: SceneItem,
  ActivePage: string
}

export type SceneItem = {
  GroupID: number | null,
  SceneID: number | null,
  SceneName: string,
  SceneTableExtensions: any
}