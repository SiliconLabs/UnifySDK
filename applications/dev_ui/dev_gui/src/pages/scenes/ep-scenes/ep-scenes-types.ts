import { SceneItem } from "../scene/scene-types"

export type EpScenesProps = {
  GroupList: any[],
  SocketServer: WebSocket,
  NodeList: any[],
  IsConnected: boolean | null,
  Storage: any,
  Unid: string,
  Ep: string
}

export type EpScenesState = {
  ProcessingScene: SceneItem
}