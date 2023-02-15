import { Image } from "./pages/ota/ota-types";
import { UPTI } from "./pages/upti/upti-list/upti-list-types";

export type AppState = {
  SocketServer: WebSocket,
  IsConnected: boolean | null,
  IsSideBarCollapsed: boolean,
  NodeList: any[],
  GroupList: any[],
  SmartStartList: any[],
  OTAImageList: Image[],
  UPTI: UPTI,
  SystemMetrics: any,
  CommissionableDevices: CommissionableDevice[],
  AppMonitoringList: any
}

export type LocalStorage = {
  Get: any,
  Set: any
}

export type CommissionableDevice = {
  Id: string,
  QRCode: string,
  DSK: string
}