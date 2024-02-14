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
            <div className={`header ${this.state.IsCollapsed ? "collapsed" : ""}`} onMouseDown={() => this.toggleCollapse(!this.state.IsCollapsed)}>
                <div className="col-sm-2 float-left margin-t-10 no-padding-r logo-container">
                    <div className="float-left">
                        <img src={logo} alt="logo" className='margin-b-10' />
                        <h4 className="logo-title">Unified IoT</h4>
                        <h4 className="logo-title">Controller</h4>
                        <span className='version'>v{localStorage.Version}{localStorage.Patch ? `-${localStorage.Patch}`: ""}</span>
                    </div>
                </div>
                <Connect ref={this.changeConnect} {...this.props} IsCollapsed={this.state.IsCollapsed} />
            </div>
        );
    }
}

export default Header