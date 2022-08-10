import { ClusterTypes } from "../../cluster-types/cluster-types"
import { NavbarItem } from "../../components/navbar/navbar-types"

export type BaseClustersProps = {
  NodeList: any[],
  GroupList: any[],
  SocketServer: WebSocket,
  IsConnected: boolean | null,
  ClusterType: ClusterTypes,
  ClusterTypeAttrs: any,
  ClusterViewOverrides: ClusterViewOverride | null,
  ShowTitle: boolean
}

export type BaseClustersState = {
  ClusterList: any[],
  ColorPicker: ColorPickerProps
}

export type ColorPickerProps = {
  ShowColorPicker: boolean,
  ColorPickerValue: any,
  ColorPickerPosition: any,
  ProcessedItem: any
}

export type ClusterViewOverride = {
  HeaderAttrNames: any | undefined,
  BodyAttrValues: any | undefined,
  ViewTable: ViewTableItem[] | undefined,
  NodesTooltip: any | undefined,
  NavbarItem: NavbarItem | undefined,
  IsExpandable: boolean | undefined
}

export type ViewTableItem = {
  Name: string,
  Value: any,
  ClassName: string | undefined
}