import { LocalStorage } from "../../app-types"

export type HeaderState = {
    IsCollapsed: boolean
}

export type HeaderProps = {
    SocketServer: WebSocket,
    IsConnected: boolean | null,
    handleIsConnectedChange: any,
    Storage: LocalStorage
}
