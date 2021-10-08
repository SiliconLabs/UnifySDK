import { ClusterTypes } from "../../cluster-types/cluster-types"
import { NavbarItem } from "../../components/navbar/navbar-types"

export type BaseClustersProps = {
  NodeList: any[],
  GroupList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null,
  ClusterType: ClusterTypes,
  ClusterTypeAttrs: any,
  ClusterViewOverrides: ClusterViewOverride | null
}

export type BaseClustersState = {
  ClusterList: any[]
}

export type ClusterViewOverride = {
  HeaderAttrNames: any | undefined,
  BodyAttrValues: any | undefined,
  ViewTable: ViewTableItem[] | undefined,
  NodesTooltip: any | undefined,
  NavbarItem: NavbarItem | undefined
}

export type ViewTableItem = {
  Name: string,
  Value: any,
  ClassName: string | undefined
}