import { Group } from "./pages/groups/groups-types";
import { Image } from "./pages/ota/ota-types";

export type AppState = {
  SocketServer: WebSocket,
  IsConnected: boolean | null,
  IsSideBarCollapsed: boolean,
  NodeList: any[],
  GroupList: Map<number, Group>,
  SmartStartList: any[],
  OTAImageList: Image[]
}

export type LocalStorage = {
  Get: any,
  Set: any
}