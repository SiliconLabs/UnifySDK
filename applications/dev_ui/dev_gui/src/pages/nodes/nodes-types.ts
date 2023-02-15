import { LocalStorage } from "../../app-types";

export const DeviceTypes = new Map<string, string>([["Any", "Any"], ["ble", "Bluetooth"], ["zb", "Zigbee"], ["zw", "ZWave"]]);

export type NodesProps = {
  NodeList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null,
  Storage: LocalStorage
}

export type NodesState = {
  NodeList: any[],
  ShowSecurityModal: boolean,
  ShowRemoveModal: boolean,
  Dsk: string,
  UnidToRemove: string,
  RequestedData: any,
  Filter: string,
  ProcessingNode: string | null,
  InclusionProccess: boolean,
  DeviceTypes: Map<string, string>;
  DeviceType: string;
  SortName: string,
  IsSortAcs: boolean,
  IsAllExpanded: boolean,
  ShowUnavailable: boolean,
  ShowOffline: boolean
}