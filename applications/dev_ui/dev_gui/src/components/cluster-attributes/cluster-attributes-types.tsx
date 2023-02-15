import { ClusterTypes } from "../../cluster-types/cluster-types"

export type ClusterAttrState = {
    Unid: string,
    SupportedAttributes: any,
    PristineSupportedAttributes: any,
    ShowModal: boolean,
    HideUnSup: boolean,
    SupportedCommands: string[],
    ForceRead: boolean
}

export type ClusterAttrProps = {
    ClusterType: ClusterTypes,
    ClusterAttr: any[],
    SocketServer: WebSocket
}