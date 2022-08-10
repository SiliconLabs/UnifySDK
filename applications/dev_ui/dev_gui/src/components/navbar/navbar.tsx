import React from 'react';
import { Link } from 'react-router-dom';
import './navbar.css';
import { NavbarItems } from './navbar-items';
import { NavbarProps, NavbarState, SideMenu } from './navbar-types';
import * as IoIcons from 'react-icons/io';
import * as MdIcons from 'react-icons/md';
import * as CgIcons from 'react-icons/cg';
import * as GrIcons from 'react-icons/gr';
import { ProSidebar, Menu, MenuItem, SubMenu } from 'react-pro-sidebar';
import 'react-pro-sidebar/dist/css/styles.css';

class Navbar extends React.Component<NavbarProps, NavbarState> {
  constructor(props: NavbarProps) {
    super(props);
    let navItem = NavbarItems.filter(i => i.path === window.location.pathname);
    this.state = {
      ActivePath: navItem.length ? navItem[0].path : '/nodes',
      ActiveMenu: null
    };
  }

  getSideMenuIcon = (menu: string) => {
    switch (menu) {
      case SideMenu.NetworkManagement:
        return <CgIcons.CgEditUnmask />
      case SideMenu.Debugging:
        return <CgIcons.CgDebug />
      case SideMenu.NodesConfiguration:
        return <MdIcons.MdSettingsApplications />
      case SideMenu.NodeManagement:
        return <GrIcons.GrConfigure />
      case SideMenu.Actuators:
        return <IoIcons.IoIosSwitch />
      case SideMenu.Sensors:
        return <MdIcons.MdSensors />
      default:
        return <CgIcons.CgEditBlackPoint />
    }
  }

  render() {
    return (
      <>
        <ProSidebar collapsed={this.props.IsSideBarCollapsed}>
          <Menu >
            {Object.keys(SideMenu).map((menuKey, indexMenu) => {
              let menu = (SideMenu as any)[menuKey];
              return (
                <SubMenu key={indexMenu} title={menu} icon={this.getSideMenuIcon(menu)} open={this.state.ActiveMenu === menu && !this.props.IsSideBarCollapsed} onClick={() => { this.setState({ ActiveMenu: menu }); this.props.ToggleSideBar(false); }}>
                  {NavbarItems.filter(i => i.subMenu === menu).sort((a, b) => a.title.localeCompare(b.title)).map((item, index) => {
                    return (
                      <MenuItem key={index} className={this.state.ActivePath === item.path ? "active" : ""} onClick={(event) => { event.stopPropagation(); this.setState({ ActivePath: item.path }) }}>
                        <Link to={item.path}>
                          <div className="nav-menu-icon">{item.icon}</div>
                          <span>{item.title}</span>
                        </Link>
                      </MenuItem>
                    );
                  })}
                </SubMenu>
              )
            })
            }
          </Menu>
        </ProSidebar>
      </>
    );
  }
}

export default Navbar;