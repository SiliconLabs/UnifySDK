import React, { Component } from 'react';
import logo from './assets/logo.png';
import initIcon from './assets/init-icon.svg';
import waitingIcon from './assets/waiting-icon.jpg';
import checkmarkIcon from './assets/checkmark-icon.svg';
import errorIcon from './assets/error-icon.svg';
import './App.css';
import { invoke } from '@tauri-apps/api/tauri'
import { ApplicationState, AppState, BuildStatus, SourceType, Status } from './app-types';
import { listen } from '@tauri-apps/api/event'

class App extends Component<{}, AppState> {
  constructor(props: {}) {
    super(props);
    this.state = {
      ApplicationState: {
        Status: Status.Init,
        BuildActions: {
          Docker: BuildStatus.Init,
          DockerCompose: BuildStatus.Init,
          UnifyLibrary: BuildStatus.Init,
          BuildDocker: BuildStatus.Init,
          RunDocker: BuildStatus.Init
        },
        ErrorMessage: ""
      } as ApplicationState,
      DeviceList: [],
      TargetDevice: "",
      SourceType: SourceType.Manual,
      Region: "EU",
      DataType: "serial"
    };
  }
  listener: any;
  deviceList: any;
  invoker: any;

  componentDidMount() {
    if (!this.listener)
      this.listener = listen('state', (event: any) => {
        console.log(event.payload);
        this.setState({ ApplicationState: event.payload });
      });
    if (!this.deviceList)
      this.deviceList = listen('devices', (event: any) => {
        if (event?.payload && event.payload.length) {
          let devices = event.payload.match(/serialNumber=[0-9]{0,32}/g).map((i: string) => i.replaceAll("serialNumber=", ""));
          this.setState({ DeviceList: devices, TargetDevice: devices && devices[0], SourceType: event.payload && event.payload.length > 0 ? SourceType.Detected : SourceType.Manual });
        }
      });
    if (!this.invoker)
      this.invoker = invoke("init").then((state: any) => {
        console.log(state);
        this.setState({ ApplicationState: state });
      });
  }

  getTitle = () => {
    switch (this.state.ApplicationState.Status) {
      case Status.Init:
        return "The portable runtime environment deploys multiple docker containers running the Unify SDK microservices within a docker network. The Unify SDK relies on multiple protocol controllers and the runtime environment currently only supports the Z-Wave protocol controller.";
      case Status.Build:
        return this.state.ApplicationState.BuildActions.BuildDocker === BuildStatus.Waiting ? "Building Unify Portable Runtime..." : "Checking system requirements...";
      case Status.SetDevice:
        return "If you are on Windows or Mac you will have to provide the serial number for your Thunderboard or WTSK board which is flashed with a Serial API. This serial number can be found in Simplicity Studio. If you are on linux you can simply provide the USB path to the device like /dev/ttyUSB0 or similar.";
      case Status.Run:
        return "Starting Unify Portable Runtime...";
      case Status.Complete:
        return "Сongratulations! Applications should boot up including the Developer GUI. The developer GUI will be the essential tool to control end devices and is available at your http://localhost:3080. When opening the developer GUI you need to connect to the Unify network by connecting to the MQTT Broker at mqtt-broker on port 1803.";
    }
  }

  isNextHidden = () => {
    return this.state.ApplicationState.Status === Status.Build || this.state.ApplicationState.Status === Status.Run || this.state.ApplicationState.Status === Status.Complete;
  }

  isNextDisabled = () => {
    return this.state.ApplicationState.Status === Status.SetDevice && !this.state.TargetDevice || this.state.ApplicationState.Status === Status.Init && !!this.state.ApplicationState.ErrorMessage;
  }

  getExitName = () => {
    return this.state.ApplicationState.Status === Status.Complete || this.state.ApplicationState.BuildActions.Docker === BuildStatus.Error || this.state.ApplicationState.BuildActions.DockerCompose === BuildStatus.Error
      || this.state.ApplicationState.BuildActions.UnifyLibrary === BuildStatus.Error || this.state.ApplicationState.BuildActions.BuildDocker === BuildStatus.Error
      || this.state.ApplicationState.BuildActions.RunDocker === BuildStatus.Error
      || this.state.ApplicationState.Status === Status.Init && !!this.state.ApplicationState.ErrorMessage? "Close" : "Cancel"
  }

  onNextClick = () => {
    switch (this.state.ApplicationState.Status) {
      case Status.Init:
        invoke('build');
        break;
      case Status.SetDevice:
        if (this.state.TargetDevice)
          invoke('run', { device: this.state.TargetDevice, sourceType: SourceType[this.state.SourceType], dataType: this.state.DataType, region: this.state.Region });
        break;
    }
  }

  getListIcon = (status: BuildStatus) => {
    switch (status) {
      case BuildStatus.Init:
        return initIcon;
      case BuildStatus.Waiting:
        return waitingIcon;
      case BuildStatus.Pass:
        return checkmarkIcon;
      case BuildStatus.Error:
        return errorIcon;
    }
  }

  onChangeType = (event: any) => {
    this.setState({ DataType: event.target.value });
  }

  onChangeSource = (event: any) => {
    this.setState({ SourceType: Number(event.target.value) });
  }

  onChangeDevice = (event: any) => {
    this.setState({ TargetDevice: event.target.value });
  }

  onChangeRegion = (event: any) => {
    this.setState({ Region: event.target.value });
  }

  getContent = () => {
    switch (this.state.ApplicationState.Status) {
      case Status.Build:
        return (<div className='list'>
          <div className='list-item'><img src={this.getListIcon(this.state.ApplicationState.BuildActions.Docker)} />Docker</div>
          <div className='list-item'><img src={this.getListIcon(this.state.ApplicationState.BuildActions.DockerCompose)} />Docker Compose</div>
          <div className='list-item'><img src={this.getListIcon(this.state.ApplicationState.BuildActions.UnifyLibrary)} />Unify Library</div>
          <div className='list-item'><img src={this.getListIcon(this.state.ApplicationState.BuildActions.BuildDocker)} />Build Docker</div>
        </div>);
      case Status.SetDevice:
        return (<div>
          <div className='margin-v-10' hidden={!this.state.DeviceList || this.state.DeviceList.length === 0}>
            <input type="radio" value={SourceType.Detected} name="SourceType" defaultChecked={this.state.SourceType === SourceType.Detected} onChange={this.onChangeSource} /> Select A Device From The List
            <input className='margin-l-20' type="radio" value={SourceType.Manual} name="SourceType" defaultChecked={this.state.SourceType === SourceType.Manual} onChange={this.onChangeSource} /> Enter Manually
          </div>
          <div className='select-group' hidden={this.state.SourceType === SourceType.Manual}>
            <label>Device:</label>
            <select name="device" onChange={this.onChangeDevice}>
              {this.state.DeviceList.map((device: any) =>
                <option value={device} key={device}>{device}</option>
              )}
            </select>
          </div>
          <div className='select-group' hidden={this.state.SourceType === SourceType.Detected}>
            <label>Type:</label>
            <select name="type" onChange={this.onChangeType} defaultValue={this.state.DataType}>
              <option value="name">Name</option>
              <option value="ip">Ip Address</option>
            </select>
          </div>
          <div hidden={this.state.SourceType === SourceType.Detected}>
            <label>Device:</label>
            <input value={this.state.TargetDevice} onChange={this.onChangeDevice} />
          </div>
          <div className='select-group'>
            <label>Region:</label>
            <select name="region" onChange={this.onChangeRegion} value={this.state.Region}>
              <option value="EU">EU</option>
              <option value="US">US</option>
              <option value="ANZ">ANZ</option>
              <option value="HK">HK</option>
              <option value="IN">IN</option>
              <option value="IL">IL</option>
              <option value="JP">JP</option>
              <option value="KR">KR</option>
              <option value="US_LR">US_LR</option>
              <option value="CN">CN</option>
              <option value="RU">RU</option>
            </select>
          </div>
        </div>);
      case Status.Run:
        return (<div className='list'>
          <div className='list-item'><img src={this.getListIcon(this.state.ApplicationState.BuildActions.RunDocker)} />Run Docker</div>
        </div>);
    }
  }

  render() {
    return (
      <div className="app" >
        <div>
          <img src={logo} className="logo" alt="logo" />
        </div>

        <div className='body'>
          <p className='title'>{this.getTitle()}</p>
          <div className='content'>{this.getContent()}</div>
          <p className='error'>{this.state.ApplicationState.ErrorMessage ? `ERROR: ${this.state.ApplicationState.ErrorMessage}` : ""}</p>
        </div>

        <div className='footer'>
          <button className='btn cancel-btn' onClick={() => invoke('exit')}>{this.getExitName()}</button>
          <button className={`btn next-btn ${this.isNextDisabled() ? "disabled" : ""}`} hidden={this.isNextHidden()} disabled={this.isNextDisabled()} onClick={this.onNextClick}>Next</button>
        </div>
      </div>
    );
  }
}

export default App;
