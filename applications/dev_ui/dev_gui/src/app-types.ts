import { Image } from "./pages/ota/ota-types";

export type AppState = {
  SocketServer: WebSocket,
  IsConnected: boolean | null,
  IsSideBarCollapsed: boolean,
  NodeList: any[],
  GroupList: any[],
  SmartStartList: any[],
  OTAImageList: Image[]
}

export type LocalStorage = {
  Get: any,
  Set: any
}