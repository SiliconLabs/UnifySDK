import React from 'react';
import { Link } from 'react-router-dom';
import './navbar.css';
import { NavbarItems } from './navbar-items';
import { NavbarProps, NavbarState } from './navbar-types';
import Tooltip from '@material-ui/core/Tooltip';

class Navbar extends React.Component<NavbarProps, NavbarState> {
  constructor(props: NavbarProps) {
    super(props);
    let navItem = NavbarItems.filter(i => i.path === window.location.pathname);
    this.state = {
      active: navItem.length ? navItem[0].path : '/nodes'
    };
  }

  render() {
    return (
      <nav className={`nav-menu`}>
        <ul className='nav-menu-items'>
          {NavbarItems.map((item, index) => {
            var liClasses = this.state.active === item.path ? 'active' : '';
            return (
              <li key={index} className={[item.cName, liClasses].join(' ')}
                onClick={() => this.setState({ active: item.path })}>
                <Tooltip title={this.props.IsSideBarCollapsed ? item.title : ""}>
                  <Link to={item.path}>
                    <div className="nav-menu-icon">{item.icon}</div>
                    <span hidden={this.props.IsSideBarCollapsed}>{item.title}</span>
                  </Link>
                </Tooltip>
              </li>
            );
          })}
        </ul>
      </nav>
    );
  }
}

export default Navbar;