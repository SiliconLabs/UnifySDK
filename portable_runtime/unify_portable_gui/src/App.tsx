import React, { Component } from 'react';
import { Form } from 'react-bootstrap';
import initIcon from './assets/init-icon.svg';
import waitingIcon from './assets/waiting-icon.jpg';
import checkmarkIcon from './assets/checkmark-icon.svg';
import './App.css';
import { invoke } from '@tauri-apps/api/tauri'
import { AppState, Frequency, Status, BuildStatus } from './app-types';
import { listen } from '@tauri-apps/api/event'
class App extends Component<{}, AppState> {
  constructor(props: {}) {
    super(props);
    this.state = {
      Status: Status.PreRequisitesCheck,
      PreRequisitesCheck: {
        Docker: BuildStatus.Init,
        DockerCompose: BuildStatus.Init,
        UnifyLibrary: BuildStatus.Init,
      },
      ProtocolList: {
        "Z-Wave": {
          Enabled: true,
          Device: '',
          IsManual: false,
          RF: Frequency.EU
        },
        "Zigbee-NCP": {
          Enabled: false,
          Device: '',
          IsManual: false
        },
        "AoX-Bluetooth": {
          Enabled: false,
          Device: '',
          IsManual: false
        },
        "MultiProtocol": {
          Enabled: false,
          Device: '',
          IsManual: false
        }
      },
      Frameworks: {
        NAL: true,
        "Image-Provider": true,
        GMS: true,
        UPVL: true
      },
      Applications: {
        UMB: false
      },
      ServiceList: [],
      DeviceList: [],
      StatusList: [],
      MultiProtocols: false,
      ErrorMessage: "",
      Conflicts: {}
    };
  }
  isComponentMount: boolean = false;

  componentDidMount() {
    if (!this.isComponentMount) {

      listen('req-check', (event: any) => {
        this.setState({ PreRequisitesCheck: event.payload, ErrorMessage: event.payload.ErrorMessage });
      });

      listen('state', (event: any) => {
        this.setState({ Status: event.payload.Status, ErrorMessage: event.payload.ErrorMessage });
      });

      listen('error', (event: any) => {
        if (event?.payload)
          this.setState({ ErrorMessage: event.payload, Status: Status.Error });
      });

      listen('status', (event: any) => {
        if (event?.payload){
          const json_array = JSON.parse(event.payload);
          this.setState({ StatusList: json_array.filter((data: any) => data.Names.startsWith("unify-")) });
        }
      });

      invoke("init").then((state: any) => {
        this.setState({ Status: state.Status, ErrorMessage: state.ErrorMessage });
      });

      this.onDiscover();

      this.isComponentMount = true;
    }
  }

  getTitle = () => {
    switch (this.state.Status) {
      case Status.PreRequisitesCheck:
        return "Requirement Check";
      case Status.Protocol:
        return "Protocol";
      case Status.Framework:
        return "Framework Services";
      case Status.Configuration:
        return "Configuration";
      case Status.Review:
        return "Selection Review";
      case Status.Running:
        return "Running Status";
      case Status.Error:
        return "Error!!!";
    }
  }

  getExitName = () => {
    return this.isExit() ? "Close" : "Back";
  }

  isExit = () => {
    return this.state.Status === Status.Protocol || this.state.Status === Status.Error || this.state.Status === Status.Running
  }

  onExitClick = () => {
    if (this.state.Status === Status.Protocol
      || this.state.Status === Status.Running
      || this.state.Status === Status.Error)
      invoke('exit').then();
    else
      this.setState({ Status: this.state.Status - 1 });
  }

  isNextHidden = () => {
    return this.state.Status === Status.Running || this.state.Status === Status.Error;
  }

  isNextDisabled = () => {
    return (this.state.Status === Status.Protocol
      && !this.state.ProtocolList["Z-Wave"].Enabled && !this.state.ProtocolList["AoX-Bluetooth"].Enabled
      && !this.state.ProtocolList["Zigbee-NCP"].Enabled && !this.state.MultiProtocols)
      || !(this.state.Status !== Status.Review || !this.state.Conflicts.length) || this.state.PreRequisitesCheck.UnifyLibrary !== BuildStatus.Pass;
  }

  onNextClick = () => {
    switch (this.state.Status) {
      case Status.PreRequisitesCheck:
        this.setState({ Status: Status.Protocol });
        break;
      case Status.Protocol:
        this.setState({ Status: Status.Framework });
        break;
      case Status.Framework:
        this.setState({ Status: Status.Configuration });
        break;
      case Status.Configuration:
        let serviceList: string[] = [];
        let protocolList: any[] = [];
        Object.keys(this.state.ProtocolList).map(i => {
          if ((this.state.ProtocolList as any)[i].Enabled) {
            serviceList.push(i);
            protocolList.push({ Protocol: i, Device: (this.state.ProtocolList as any)[i].Device, IsManual: (this.state.ProtocolList as any)[i].IsManual });
          }
        });
        Object.keys(this.state.Frameworks).map(i => {
          if ((this.state.Frameworks as any)[i])
            serviceList.push(i);
        });
        Object.keys(this.state.Applications).map(i => {
          if ((this.state.Applications as any)[i])
            serviceList.push(i);
        });
        this.setState({ ServiceList: serviceList });
        invoke('validate', { serviceList: serviceList.join(','), protocolList: protocolList, region: this.state.ProtocolList['Z-Wave'].RF }).then((res: any) => {
          if (res && res.length) {
            let conflicts: any = {};
            res.forEach((item: any) => {
              conflicts[item[0]] = item[1];
            });
            this.setState({ Conflicts: conflicts });
          }
        });
        this.setState({ Status: Status.Review });
        break;
      case Status.Review:
        if (!this.state.Conflicts.length) {
          invoke('run', { device: this.state.ProtocolList['Z-Wave'].Device, sourceType: this.state.ProtocolList['Z-Wave'].IsManual ? "Manual" : "Detected" });
          this.setState({ Status: Status.Running });
        }
        break;
      case Status.Running:
        this.setState({ Status: Status.Error });
        break;
    }
  }

  onProtocolChange = (event: any) => {
    let protocol = this.state.ProtocolList;

    if (event.target.name === "MultiProtocols") {
      if (event.target.checked) {
        protocol["MultiProtocol"].Enabled = true;
        protocol["Zigbee-NCP"].Enabled = false;
      } else {
        protocol["MultiProtocol"].Enabled = false;
      }
      this.setState({ MultiProtocols: event.target.checked });
    } else {
      (protocol as any)[event.target.name].Enabled = event.target.checked;

      if (event.target.name === "MultiProtocol" && event.target.checked)
        protocol["Zigbee-NCP"].Enabled = false;

      if (event.target.name === "Zigbee-NCP" && event.target.checked)
        protocol["MultiProtocol"].Enabled = false;

      if (!protocol["MultiProtocol"].Enabled)
        this.setState({ MultiProtocols: false });
    }
    this.setState({ ProtocolList: protocol });
  }

  onFrameworkChange = (event: any) => {
    let frameworks = this.state.Frameworks;
    (frameworks as any)[event.target.name] = event.target.checked;
    this.setState({ Frameworks: frameworks });
  }

  onApplicationChange = (event: any) => {
    let applications = this.state.Applications;
    (applications as any)[event.target.name] = event.target.checked;
    this.setState({ Applications: applications });
  }

  onDeviceChange = (name: string, isCheckbox: boolean, event: any) => {
    let protocolList = this.state.ProtocolList as any;
    protocolList[name][event.target.name] = isCheckbox ? event.target.checked : event.target.value;
    this.setState({ ProtocolList: protocolList });

    let optList = document.getElementsByName("Device");
    for (let item of optList) {
      if (item instanceof HTMLSelectElement) {
        for (let option of item.options) {
          if (item.id !== name) {
            if(option.value === event.target.value && option.value !== "") {
              option.disabled = true;
            } else {
              option.disabled = false;
            }
          }
        }
      }
    }
  }

  onDiscover = () => {
    invoke("discover").then((devices: any) => {
      if (devices && devices.length) {
        let deviceList = devices.match(/serialNumber=[0-9]{0,32}/g)?.map((i: string) => i.replaceAll("serialNumber=", ""));
        this.setState({ DeviceList: deviceList });
      }
    });
  }

  getListIcon = (status: BuildStatus) => {
    switch (status) {
      case BuildStatus.Init:
        return initIcon;
      case BuildStatus.Waiting:
        return waitingIcon;
      case BuildStatus.Pass:
        return checkmarkIcon;
    }
  }

  getContent = () => {
    switch (this.state.Status) {
      case Status.PreRequisitesCheck:
        return (<div className='col-sm-12 req-check'>
            <div className="check-container">
              <div className='list-item'>Docker <img src={this.getListIcon(this.state.PreRequisitesCheck.Docker)} /></div>
              <div className='list-item'>Docker Compose <img src={this.getListIcon(this.state.PreRequisitesCheck.DockerCompose)} /></div>
              <div className='list-item'>Unify Library <img src={this.getListIcon(this.state.PreRequisitesCheck.UnifyLibrary)} /></div>
            </div>
          </div>);
      case Status.Protocol:
        return (<div className='col-sm-12 protocol'>
          <Form.Label column>
            <span className='check-label'>Z-Wave</span>
            <div className="check-container">
              <Form.Check name="Z-Wave" checked={this.state.ProtocolList["Z-Wave"].Enabled} onChange={this.onProtocolChange} />
            </div>
          </Form.Label>
          <Form.Label column>
            <span className='check-label'>Zigbee(NCP)</span>
            <div className="check-container">
              <Form.Check name="Zigbee-NCP" checked={this.state.ProtocolList["Zigbee-NCP"].Enabled} onChange={this.onProtocolChange} />
            </div>
          </Form.Label>
          <Form.Label column>
            <span className='check-label'>Bluetooth AoX</span>
            <div className="check-container">
              <Form.Check name="AoX-Bluetooth" checked={this.state.ProtocolList["AoX-Bluetooth"].Enabled} onChange={this.onProtocolChange} />
            </div>
          </Form.Label>
          <Form.Label column>
            <span className='check-label'>MultiProtocols</span>
            <div className="check-container">
              <Form.Check name="MultiProtocols" checked={this.state.MultiProtocols} onChange={this.onProtocolChange} />
            </div>
          </Form.Label> 
          <div className={!this.state.MultiProtocols ? "disabled" : ""}>
            <Form.Label column>
              <span className='check-label small'>Zigbee(RCP)</span>
              <div className="check-container">
                <Form.Check name="MultiProtocol" checked={this.state.ProtocolList["MultiProtocol"].Enabled} onChange={this.onProtocolChange} disabled={!this.state.MultiProtocols} />
              </div>
            </Form.Label>
          </div>
        </div>);
      case Status.Framework:
        return (<>
          <div className='col-sm-12 framework'>
            <Form.Label column>
              <span className='check-label'>NAL</span>
              <div className="check-container">
                <Form.Check name="NAL" defaultChecked={this.state.Frameworks.NAL} onChange={this.onFrameworkChange} />
              </div>
            </Form.Label>
            <div className='description'>Name and Location Service</div>
          </div>
          <div className='col-sm-12 framework'>
            <Form.Label column>
              <span className='check-label'>Image Provider</span>
              <div className="check-container">
                <Form.Check name="Image-Provider" defaultChecked={this.state.Frameworks["Image-Provider"]} onChange={this.onFrameworkChange} />
              </div>
            </Form.Label>
            <div className='description'>Image Provider Service for OTA</div>
          </div>
          <div className='col-sm-12 framework'>
            <Form.Label column>
              <span className='check-label'>GMS</span>
              <div className="check-container">
                <Form.Check name="GMS" defaultChecked={this.state.Frameworks.GMS} onChange={this.onFrameworkChange} />
              </div>
            </Form.Label>
            <div className='description'>Group Management Service</div>
          </div>
          <div className='col-sm-12 framework'>
            <Form.Label column>
              <span className='check-label'>UPVL</span>
              <div className="check-container">
                <Form.Check name="UPVL" defaultChecked={this.state.Frameworks.UPVL} onChange={this.onFrameworkChange} />
              </div>
            </Form.Label>
            <div className='description'>Smart Start Management Service</div>
          </div>

        <div>
          <p className='title'>Applications</p>

          <div className='col-sm-12 framework'>
            <Form.Label column>
              <span className='check-label'>UMB</span>
              <div className="check-container">
                <Form.Check name="UMB" defaultChecked={this.state.Applications.UMB} onChange={this.onApplicationChange} />
              </div>
            </Form.Label>
            <div className='description'>Unify Matter Bridge</div>
          </div>
          <div className='col-sm-12 framework'>
            <Form.Label column>
              <span className='check-label'>Dev-GUI</span>
              <div className="check-container">
                <Form.Check name="Dev-GUI" checked disabled />
              </div>
            </Form.Label>
            <div className='description'>Developer GUI</div>
          </div>
        </div>
        </>);
      case Status.Configuration:
        return (
          <>
            <div className='d-inline-block mb-2'>
              <button className='btn cancel-btn' onClick={this.onDiscover}>Discover Device</button>
            </div>
            <div className='configuration'>
              {Object.keys(this.state.ProtocolList).map((protocol, index) => {
                let item = (this.state.ProtocolList as any)[protocol];
                if (!item.Enabled)
                  return;
                return (<div className='config-item' key={index}>
                  <div className='config-item-label col-12'>{protocol}</div>
                  <div className='config-device'>
                    <Form.Label className='col-3'>Device</Form.Label>
                    {!item.IsManual
                      ? <Form.Control as="select" name="Device" id={protocol} className='col-6' onChange={this.onDeviceChange.bind(this, protocol, false)} value={item.Device}>
                        <option hidden={!item.Device}></option>
                        {this.state.DeviceList?.map((device: any, index: number) => {
                          return <option key={index} value={device}>{device}</option>
                        })}
                      </Form.Control>
                      : <Form.Control type="text" className='col-6' name="Device" value={item.Device} onChange={this.onDeviceChange.bind(this, protocol, false)} />
                    }
                    <Form.Label className='col-3 manual'>
                      <span className='check-label'>manual</span>
                      <Form.Check name="IsManual" onChange={this.onDeviceChange.bind(this, protocol, true)} checked={item.IsManual} />
                    </Form.Label>
                  </div>
                  <div className='config-device' hidden={protocol !== "Z-Wave"}>
                    <Form.Label className='col-3'>RF Region</Form.Label>
                    <Form.Control as="select" name="RF" className='col-6' onChange={this.onDeviceChange.bind(this, protocol, false)} value={item.RF}>
                      {Object.keys(Frequency).map((fr: any, index: number) => {
                        return <option key={index} value={fr}>{fr}</option>
                      })}
                    </Form.Control>
                  </div>
                </div>)
              })}
            </div>
          </>);
      case Status.Review:
        return (<div className='review'>
          <div className='table-header'>
            <div className='col-3'>Selection</div>
            <div className='col-6'>Configuration</div>
            <div className='col-3'>Conflicts</div>
          </div>
          {Object.keys(this.state.ProtocolList).map((protocol, index) => {
            let item = (this.state.ProtocolList as any)[protocol];
            if (item.Enabled)
              return (<div className='table-container' key={index}>
                <div className='table-item' >
                  <div className='col-3'>{protocol}</div>
                  <div className='col-6'>
                    <div className='col-12'>Device: {item.Device}</div>
                    <div hidden={protocol !== "Z-Wave"} className='col-12'>RF: {item.RF}</div>
                  </div>
                  <div className='col-3'>{this.state.Conflicts[protocol] ?? "NO"}</div>
                </div>
              </div>)
          })}
          {Object.keys(this.state.Frameworks).map((framework, index) => {
            let item = (this.state.Frameworks as any)[framework];
            if (item)
              return (<div className='table-container' key={index}>
                <div className='table-item' >
                  <div className='col-3'>{framework}</div>
                  <div className='col-6'></div>
                  <div className='col-3'>{this.state.Conflicts[framework] ?? "NO"}</div>
                </div>
              </div>)
          })}
        </div>);
      case Status.Running:
        return (
          <div className='review'>
            {this.state.StatusList.length === 0 ? (
              <div className='justify-content-center align-items-center spinner-container'>
                <div className='text-center'>
                  <div className='spinner-border text-primary' role='status'>
                    <span className='sr-only'>Loading...</span>
                  </div>
                  <p className='mt-2'>Loading...</p>
                </div>
            </div>
            ) : (
              <div>
                <div className='table-header'>
                  <div className='col-5'>Service</div>
                  <div className='col-3'>State</div>
                  <div className='col-4'>Status</div>
                </div>
                {this.state.StatusList.map((service, index) => {
                  return (
                  <div className='table-container' key={index}>
                    <div className='table-item'>
                      <div className='col-5'>{service.Names}</div>
                      <div className='col-3'>{service.State}</div>
                      <div className='col-4'>{service.Status}</div>
                    </div>
                  </div>
                );
                })}
              </div>
            )}
          </div>
        );
      case Status.Error:
        return (<div className='error'>{this.state.ErrorMessage}</div>);
    }
  }

  render() {
    return (
      <div className="app" >

        <div className='body'>
          <p className='title'>{this.getTitle()}</p>
          <div className='content'>{this.getContent()}</div>
        </div>

        <div className='footer'>
          <button className={`btn cancel-btn ${this.isExit() ? "float-right" : ""}`} onClick={this.onExitClick} hidden={this.state.Status === Status.Protocol || this.state.Status === Status.PreRequisitesCheck}>{this.getExitName()}</button>
          <button className={`btn next-btn ${this.isNextDisabled() ? "disabled" : ""}`} hidden={this.isNextHidden()} disabled={this.isNextDisabled()} onClick={this.onNextClick}>{this.state.Status === Status.Review ? "Run" : "Next"}</button>
        </div>

      </div>
    );
  }
}

export default App;
