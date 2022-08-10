export type SystemMetricsListProps = {
  SystemMetricsList: any,
  SocketServer: WebSocket,
  IsConnected: boolean | null
}

export type SystemMetricsListState = {
  ShowModal: boolean,
  CurrentMid: string,
  CurrentInterval: number
}