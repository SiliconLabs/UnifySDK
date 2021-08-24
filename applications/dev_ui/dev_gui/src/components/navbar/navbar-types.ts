export type NavbarState = {
  active: string
}

export type NavbarProps = {
  ToggleSideBar: any,
  IsSideBarCollapsed: boolean
}

export type NavbarItem = {
  name: string,
  title: string,
  path: string,
  icon: any,
  cName: string
}