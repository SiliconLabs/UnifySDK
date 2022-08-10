import { ClusterTypes } from "../../cluster-types/cluster-types"
import { Group } from "../../pages/groups/groups-types"

export type EditGroupDlgState = {
    ProcessingGroup: Group,
    PristineGroupName: string,
    EpExist: boolean,
    IsGroupExist:boolean,
    EndpointList: any[],
    Filter: string,
    ShowModal: boolean,
    CheckedAll: boolean,
    ClusterType: ClusterTypes | null,
    Updates: any,
    IsNew: boolean,
    GroupIds: number[]
}

export type EditGroupDlgProps = {
    SocketServer: WebSocket,
    NodeList: any[]
}