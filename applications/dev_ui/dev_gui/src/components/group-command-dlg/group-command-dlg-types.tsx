import { Group } from "../../pages/groups/groups-types"

export type GroupCommandDlgState = {
    ClusterType: string,
    Command: any,
    Group: Group,
    AvailableClusters: string[],
    AvailableCommands: any[],
    ShowModal: boolean
}

export type GroupCommandDlgProps = {
    SocketServer: WebSocket
}