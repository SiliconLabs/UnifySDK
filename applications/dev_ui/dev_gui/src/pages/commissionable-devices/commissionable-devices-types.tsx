import { CommissionableDevice } from "../../app-types"

export type CommissionableDevicesProps = {
  List: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type CommissionableDevicesState = {
  ShowModal: boolean,
  ProcessedItem: any
}