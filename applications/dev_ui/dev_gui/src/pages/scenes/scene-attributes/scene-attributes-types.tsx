import { SceneItem } from "../scene/scene-types"

export type SceneAttributesState = {
    Scene: SceneItem
}

export type SceneAttributesProps = {
    Scene: SceneItem,
    Cluster: string,
    IsEdit: boolean,
    RemoveAttribute: any
}