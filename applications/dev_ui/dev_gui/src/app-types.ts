import { Image } from "./pages/ota/ota-types";
import { UPTI } from "./pages/upti/upti-list/upti-list-types";

export type AppState = {
  SocketServer: WebSocket,
  IsConnected: boolean | null, //null - transition state. used to determine when a connection has been initiated, but confirmation has not yet been received.
  IsSideBarCollapsed: boolean,
  NodeList: any[],
  GroupList: any[],
  SmartStartList: any[],
  OTAImageList: Image[],
  UPTI: UPTI,
  SystemMetrics: any,
  CommissionableDevices: CommissionableDevice[],
  AppMonitoringList: any,
  ShowRedirectModal:boolean,
  ShowRedirectSpinner: boolean
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