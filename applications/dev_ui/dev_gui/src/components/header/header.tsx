import * as React from 'react';
import Connect from '../connect/connect';
import logo from '../../assets/logo.png';
import './header.css';
import { HeaderProps, HeaderState } from './header-types';


class Header extends React.Component<HeaderProps, HeaderState> {
    constructor(props: HeaderProps) {
        super(props);
        this.state = {
            IsCollapsed: props.IsConnected || true
        }
        this.changeConnect = React.createRef();
        this.toggleCollapse = this.toggleCollapse.bind(this);
    }

    changeConnect: any;

    toggleCollapse(isCollapsed: boolean | null = null) {
        this.setState({ IsCollapsed: isCollapsed === null ? !this.state.IsCollapsed : isCollapsed });
    }

    render() {
        return (
            <div className={`header ${this.state.IsCollapsed ? "collapsed" : ""}`} onMouseDown={() => this.toggleCollapse(null)}>
                <div className={`${this.state.IsCollapsed ? "col-sm-2" : "col-sm-2"} float-left margin-t-10`}>
                    <div className="float-left">
                        <img src={logo} alt="logo" />
                        <h4 hidden={this.state.IsCollapsed}>Unified IoT Controller</h4>
                    </div>
                </div>
                <Connect ref={this.changeConnect} {...this.props} IsCollapsed={this.state.IsCollapsed} />
            </div>
        );
    }
}

export default Header