import { LocalStorage } from "../../app-types"

export type ConnectionState = {
  Host: string;
  Port: number;
  Status: string;
  IsError: boolean;
  TLS: boolean;
  CA: any;
  CAName: any;
  ClientKey: any;
  ClientKeyName: any;
  ClientCertificate: any;
  ClientCertificateName: any;
}

export type ConnectionProps = {
  SocketServer: WebSocket,
  IsConnected: boolean | null,
  IsCollapsed: boolean,
  handleIsConnectedChange: any,
  Storage: LocalStorage
}
