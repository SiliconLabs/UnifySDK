import { ClusterTypes } from "../../cluster-types/cluster-types"
import { Group } from "../../pages/groups/groups-types"

export type EditGroupDlgState = {
    ProcessingGroup: Group,
    PristineGroupName: string,
    IsGroupValid: boolean,
    EndpointList: any[],
    ShowModal: boolean,
    CheckedAll: boolean,
    ClusterType: ClusterTypes | null
}

export type EditGroupDlgProps = {
    SocketServer: WebSocket,
    NodeList: any[]
}