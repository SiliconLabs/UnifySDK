export type NavbarState = {
  ActivePath: string,
  ActiveMenu: string | null
}

export type NavbarProps = {
  ToggleSideBar: any
  IsSideBarCollapsed: boolean
}

export type NavbarItem = {
  name: string,
  title: string,
  path: string,
  icon: any,
  cName: string,
  subMenu: string
}

export enum SideMenu {
  NetworkManagement = "Network Management",
  Debugging = "Debugging",
  NodesConfiguration = "Nodes Configuration",
  NodeManagement = "Node Management",
  Actuators = "Actuators",
  Sensors = "Sensors"
}