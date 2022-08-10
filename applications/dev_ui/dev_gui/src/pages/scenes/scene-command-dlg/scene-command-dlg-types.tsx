import { SceneItem } from "../scene-list/scene-list-types"

export type SceneCommandDlgState = {
    ClusterType: string,
    Command: any,
    AvailableCommands: any[],
    ShowModal: boolean
}

export type SceneCommandDlgProps = {
    Scene: SceneItem,
    SocketServer: WebSocket
}