export type OTAProps = {
  NodeList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type OTAState = {
  OTAImageList: Image[],
  NodeList: any[]
}

export type Image = {
  Filename: string,
  Uiid: string,
  Unid: string[],
  Version: string,
  ApplyAfter: string,
  Md5: string
}