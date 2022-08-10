import { SceneItem } from "../scene-list/scene-list-types";

export type SceneProps = {
  SceneList: any;
  GroupList: any[];
  IsConnected: boolean | null;
  SocketServer: WebSocket;
  GroupID: string;
  SceneID: string;
}

export type SceneState = {
  Scene: SceneItem,
  Cluster: string,
  Cmd: string,
  GetScene: any,
  IsSceneListReceived: boolean
}