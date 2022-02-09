export type UPTIListProps = {
  UPTI: UPTI,
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type UPTIListState = {
  IsDiscover: boolean
  ShowModal: boolean;
  EnablePTI: EnablePTI;
}

export type EnablePTI = {
  SerialNumber: string;
  OTAEnable: boolean;
  AdditionalDataEnable: boolean;
}

export type UPTI = {
  List: UPTIItem[],
  Trace: any
}

export type UPTIItem = {
  PTIAddress: string,
  SerialNumber: string,
  Enabled: boolean
}