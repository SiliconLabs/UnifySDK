import { ClusterTypes } from "../../cluster-types/cluster-types"

export type ScenesDlgState = {
    ShowModal: boolean,
    Unid: string,
    Scenes: Scene[],
    ProcessingScene: Scene | null
}

export type ScenesDlgProps = {
    ClusterType: ClusterTypes,
    SocketServer: WebSocket,
    SceneAttrs: any[]
}

export type Scene = {
    GroupId: number,
    SceneID: number,
    TransitionTime: number,
    TransitionTime100ms: number,
    SceneName: string,
    SceneTableExtensions: any
}